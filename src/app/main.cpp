#include <spdlog/spdlog.h>

void print_version() {
    spdlog::info("Bouillabaisse version {}", VERSION);
}

int main(int argc, char* argv[]) {
    print_version();
    return 0;
}