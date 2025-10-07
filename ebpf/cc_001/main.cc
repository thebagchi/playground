#include <iostream>
#include <bpf/libbpf.h>
#include <bpf/bpf.h>
#include <csignal>
#include <unistd.h>
#include <string>
#include <cstring>

static bool running = true;
static constexpr size_t MAX_ERROR_BUFFER_SIZE = 4096;

// Helper function to print error safely
void print_libbpf_error(const std::string& context, int err) {
    size_t buffer_size = 256;
    std::string error_message;
    do {
        error_message.resize(buffer_size);
        auto result = libbpf_strerror(err, error_message.data(), error_message.size());
        if (result == 0) {
            std::cerr << context << ": " << error_message.c_str() << std::endl;
            return;
        }
        buffer_size = buffer_size  * 2;
        
    } while (buffer_size <= MAX_ERROR_BUFFER_SIZE);
    std::cerr << context << ": Unknown error (" << err << ")" << std::endl;
}

void handle_signal(int sig) {
    running = false;
}

int main() {
    struct bpf_object *obj = bpf_object__open("bpf.o");
    if (!obj) {
        print_libbpf_error("failed to open eBPF object", errno);
        return 1;
    }

    if (bpf_object__load(obj)) {
        print_libbpf_error("failed to load eBPF program", errno);
        bpf_object__close(obj);
        return 1;
    }

    struct bpf_program *prog = bpf_object__find_program_by_name(obj, "kprobe_execve");
    if (!prog) {
        std::cerr << "failed to find eBPF program" << std::endl;
        bpf_object__close(obj);
        return 1;
    }

    struct bpf_link *link = bpf_program__attach(prog);
    if (!link) {
        print_libbpf_error("failed to attach kprobe", errno);
        bpf_object__close(obj);
        return 1;
    }

    // Find map
    struct bpf_map *map = bpf_object__find_map_by_name(obj, "syscall_count");
    if (!map) {
        std::cerr << "failed to find map" << std::endl;
        bpf_link__destroy(link);
        bpf_object__close(obj);
        return 1;
    }
    int map_fd = bpf_map__fd(map);

    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);

    std::cout << "counting system calls ..." << std::endl;

    // Poll map for counts
    uint32_t key = 0;
    uint64_t count;
    while (running) {
        if (bpf_map_lookup_elem(map_fd, &key, &count) == 0) {
            std::cout << "\n count: " << count << std::flush;
        } else {
            print_libbpf_error("failed to read map", errno);
        }
        sleep(1);
    }

    // Cleanup
    std::cout << "\n final count: " << count << std::endl;
    bpf_link__destroy(link);
    bpf_object__close(obj);
    return 0;
}