#include "io/Alsa.hpp"
#include <spdlog/spdlog.h>

#include <iostream>

void print_version() {
    spdlog::info("Bouillabaisse version \"{}\"", VERSION);
}

int main(int argc, char *argv[]) {
    print_version();
    spdlog::info("TODO: the whole thing :skull:");

    AudioDeviceManager adm;

    auto input_devices = adm.get_input_devices();

    if (input_devices.empty()) {
        spdlog::error("No input audio devices found.");
        return 1;
    }

    for (const auto &device : input_devices) {
        spdlog::info("Input Device: {} (Card: {}, Device: {})",
                     device.get_dev_name(), device.get_card_name(),
                     device.get_dev_number());
    }

    std::cout << "Select device index: ";
    size_t choice;
    std::cin >> choice;

    if (choice >= input_devices.size()) {
        std::cerr << "Invalid choice" << std::endl;
        return 1;
    }

    snd_pcm_t *handle;
    if (input_devices[choice].open_stream(&handle) == 0) {
        std::cout << "Opened stream successfully! You can now read from it."
                  << std::endl;

        // Example: read small buffer (blocking)
        const int frames = 128;
        short buffer[frames * 2]; // stereo
        int err = snd_pcm_readi(handle, buffer, frames);
        if (err > 0) {
            std::cout << "Captured " << err << " frames." << std::endl;
        } else {
            std::cerr << "Read error: " << snd_strerror(err) << std::endl;
        }

        snd_pcm_close(handle);
    }

    return 0;
}
