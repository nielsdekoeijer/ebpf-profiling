#include "vmlinux.h"
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_tracing.h>
#include <bpf/usdt.bpf.h>
#include "common.h" 

struct {
    __uint(type, BPF_MAP_TYPE_RINGBUF);
    __uint(max_entries, 256 * 1024);
} rb SEC(".maps");

SEC("usdt")
int handle_checkpoint(struct pt_regs *ctx) {
    struct event_t *event;
    event = bpf_ringbuf_reserve(&rb, sizeof(*event), 0);
    if (!event) return 0;

    // get time in ns
    event->ts = bpf_ktime_get_ns();

    // store program pid
    event->pid = bpf_get_current_pid_tgid() >> 32;

    // we manually read the first argument (index 0) from the registers
    long tmp_seq;
    bpf_usdt_arg(ctx, 0, &tmp_seq);
    event->seq_id = (unsigned long long)tmp_seq;

    // get the cookie for id-ing the event source
    event->cookie_id = bpf_get_attach_cookie(ctx);

    // submit
    bpf_ringbuf_submit(event, 0);
    return 0;
}

char LICENSE[] SEC("license") = "Dual BSD/GPL";
