#include "vmlinux.h"
#include <bpf/bpf_helpers.h>

char _license[] SEC("license") = "MIT";

struct {
    __uint(type, BPF_MAP_TYPE_ARRAY);
    __type(key, __u32);
    __type(value, __u64);
    __uint(max_entries, 1);
} syscall_count SEC(".maps");

// SEC("kprobe/sys_execve")
SEC("kprobe/__x64_sys_execve")
int kprobe_execve(void *ctx) {
    __u32 key = 0;
    __u64 *val = bpf_map_lookup_elem(&syscall_count, &key);
    if (val) {
        __sync_fetch_and_add(val, 1);
    }
    return 0;
}

