#!/bin/sh

# Exit on error
set -e

# Make vmlinuz.h
bpftool btf dump file /sys/kernel/btf/vmlinux format c > vmlinux.h

# Build the main application swarm with USDT probes
g++ source.cpp -o source
g++ processor.cpp -o processor
g++ sink.cpp -o sink

# Build the eBPF program
clang -g -O2 -target bpf -c profiler.bpf.c -o profiler.bpf.o -D__TARGET_ARCH_x86 $NIX_CFLAGS_COMPILE 

# Generate the skeleton
bpftool gen skeleton profiler.bpf.o > profiler.skel.h

# Build the loader application
g++ -std=c++20 loader.cpp -o loader -lbpf

# Run our swarm! ahh
./source &
./processor &
./sink &


# Run the loader program
sudo ./loader 
