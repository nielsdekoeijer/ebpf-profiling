#include <bpf/libbpf.h>
#include <dirent.h>
#include <fcntl.h>
#include <libgen.h>
#include <unistd.h>
#include <algorithm>
#include <csignal>
#include <cstring>
#include <filesystem>
#include <format>
#include <iostream>
#include <map>
#include <string>

#include "common.h"
#include "profiler.skel.h"

const std::string probeProviderName = "buffer_tracker";
const std::string probeBegMeasurement = "beg_global";
const std::string probeEndMeasurement = "end_global";

const auto probeList = {
    probeBegMeasurement,
    std::string("beg_awe"),
    std::string("end_awe"),
    std::string("beg_writing"),
    std::string("end_writing"),
    probeEndMeasurement
};

struct Probe {
    std::size_t cookieId;
    int pid;
    std::string programName;
    std::string providerName;
    std::string probeName;
};

static auto probes = std::map<std::size_t, Probe> {};

int handle_event(void* ctx, void* data, size_t data_sz)
{
    const struct event_t* e = (const struct event_t*)data;
    auto probe = probes.at(e->cookie_id);

    std::cerr << std::format(
        "[{:>16}] [PID:{:>7}] {:<12} {:<30} | Seq: {}",
        e->ts,
        e->pid,
        probe.programName,
        std::format("{}:{}", probe.providerName, probe.probeName),
        e->seq_id)
              << std::endl;

    return 0;
}

static volatile bool exiting = false;
void sig_handler(int sig)
{
    exiting = true;
}

int main()
{
    libbpf_set_print(NULL);

    // setup the bpf program
    struct profiler_bpf* skel = profiler_bpf__open();
    if (!skel) {
        std::cerr << "Failed to setup BPF skeleton" << std::endl;
        return 1;
    }

    // lower it into the kernel
    if (profiler_bpf__load(skel)) {
        std::cerr << "Failed to load BPF skeleton" << std::endl;
        return 1;
    }

    // we track the "cookies", an id we register inside the ebpf program to identify a USDT probe
    std::size_t cookieIdCounter = 0;

    // we will investigate all open processes and try to load known USDT probes
    const std::filesystem::path procDirectory("/proc");
    for (const auto& path : std::filesystem::directory_iterator(procDirectory)) {
        // processes are always folders, else continue
        if (!path.is_directory()) {
            continue;
        }

        // ensure filename is all digits, else continue
        std::string pid_str = path.path().filename().string();
        if (!std::all_of(pid_str.begin(), pid_str.end(), ::isdigit)) {
            continue;
        }

        // we know its a number, grab the pid
        auto pid = std::stoi(pid_str);
        auto exeSymlink = path.path() / "exe";

        // if the exe symlink doesnt exist, continue
        if (!std::filesystem::exists(exeSymlink)) {
            continue;
        }

        // if the exe symlink isnt a symlink, continue
        if (!std::filesystem::is_symlink(exeSymlink)) {
            continue;
        }

        // now we can read it
        auto exeRealPath = std::filesystem::read_symlink(exeSymlink);

        // now we will try to find our known USDT probes
        auto exeRealFileName = exeRealPath.filename().string();
        auto exeRealFilePath = exeRealPath.string();

        // for each probe type...
        for (auto& probeName : probeList) {
            // create an option to pass to bpf
            LIBBPF_OPTS(bpf_usdt_opts, opts);
            opts.usdt_cookie = cookieIdCounter;

            // try to make the link
            struct bpf_link* link = bpf_program__attach_usdt(
                skel->progs.handle_checkpoint, pid, exeRealFilePath.c_str(),
                probeProviderName.c_str(), probeName.c_str(), &opts);

            // found!
            if (link) {
                auto cookieId = cookieIdCounter++;
                probes[cookieId] = Probe {
                    .cookieId = cookieId,
                    .pid = pid,
                    .programName = exeRealFileName,
                    .providerName = probeProviderName,
                    .probeName = probeName,
                };

                std::cerr << std::format("[{:>4}] Found probe {:<35} in '{}'\n",
                    cookieId, probeName, exeRealFileName);
            }
        }
    }

    // create ring buffer
    struct ring_buffer* rb = ring_buffer__new(bpf_map__fd(skel->maps.rb), handle_event, NULL, NULL);
    if (!rb) {
        std::cerr << "Failed to create ring buffer" << std::endl;
        return 1;
    }

    std::cerr << "\nTrace started. Ctrl-C to stop.\n"
              << std::endl;
    signal(SIGINT, sig_handler);

    while (!exiting) {
        int err = ring_buffer__poll(rb, 100);
        if (err < 0 && err != -EINTR) {
            fprintf(stderr, "Error polling ring buffer: %d\n", err);
            break;
        }
    }

    // Cleanup
    ring_buffer__free(rb);
    profiler_bpf__destroy(skel);
    return 0;
}
