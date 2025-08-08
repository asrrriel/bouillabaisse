#include "io/Alsa.hpp"
#include <spdlog/spdlog.h>

#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <termios.h>
#include <unistd.h>

void print_version() {
    spdlog::info("Bouillabaisse version \"{}\"", VERSION);
}

void write_wav_header(std::ofstream &file, uint32_t sample_rate,
                      uint16_t channels, uint32_t data_size) {
    file.write("RIFF", 4);
    file.write(reinterpret_cast<const char *>(&data_size),
               4); // Chunk size: 36 + data_size
    file.write("WAVE", 4);
    file.write("fmt ", 4);
    uint32_t subchunk1_size = 16;
    file.write(reinterpret_cast<const char *>(&subchunk1_size), 4);
    uint16_t audio_format = 1; // PCM
    file.write(reinterpret_cast<const char *>(&audio_format), 2);
    file.write(reinterpret_cast<const char *>(&channels), 2);
    file.write(reinterpret_cast<const char *>(&sample_rate), 4);
    uint32_t byte_rate =
        sample_rate * channels * 2; // 2 bytes per sample (16-bit)
    file.write(reinterpret_cast<const char *>(&byte_rate), 4);
    uint16_t block_align = channels * 2;
    file.write(reinterpret_cast<const char *>(&block_align), 2);
    uint16_t bits_per_sample = 16;
    file.write(reinterpret_cast<const char *>(&bits_per_sample), 2);
    file.write("data", 4);
    file.write(reinterpret_cast<const char *>(&data_size), 4);
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

        int err = snd_pcm_start(handle);
        if (err < 0) {
            std::cerr << "Capture start error: " << snd_strerror(err)
                      << std::endl;
            snd_pcm_close(handle);
            return 1;
        }

        std::vector<short> audio_buffer;
        const int frames = 128;
        short temp_buffer[frames * 2]; // Stereo
        char ch;

        struct termios ttystate1;
        tcgetattr(STDIN_FILENO, &ttystate1);
        ttystate1.c_lflag     &= ~(ICANON | ECHO);
        ttystate1.c_cc[VMIN]   = 0;
        ttystate1.c_cc[VTIME]  = 0;
        tcsetattr(STDIN_FILENO, TCSANOW, &ttystate1);
        fcntl(STDIN_FILENO, F_SETFL, fcntl(STDIN_FILENO, F_GETFL) | O_NONBLOCK);
        std::cout << "Recording... Press 'a' to stop." << std::endl;

        while (true) {
            if (read(STDIN_FILENO, &ch, 1) > 0 && ch == 'a') {
                break;
            }

            err = snd_pcm_readi(handle, temp_buffer, frames);
            if (err < 0) {
                std::cerr << "Read error: " << snd_strerror(err) << std::endl;
                break;
            }
            audio_buffer.insert(audio_buffer.end(), temp_buffer,
                                temp_buffer + err * 2);
        }

        snd_pcm_drop(handle);
        snd_pcm_close(handle);

        std::ofstream wav_file("output.wav", std::ios::binary);
        if (!wav_file.is_open()) {
            std::cerr << "Failed to open output.wav for writing" << std::endl;
            return 1;
        }

        uint32_t sample_rate = 44100;
        uint16_t channels    = 2;
        uint32_t data_size   = audio_buffer.size() * sizeof(short);
        write_wav_header(wav_file, sample_rate, channels, data_size);
        wav_file.write(reinterpret_cast<const char *>(audio_buffer.data()),
                       data_size);
        wav_file.close();

        std::cout << "Saved recording to output.wav" << std::endl;

        struct termios ttystate;
        tcgetattr(STDIN_FILENO, &ttystate);
        ttystate.c_lflag |= ICANON | ECHO;
        tcsetattr(STDIN_FILENO, TCSANOW, &ttystate);
    } else {
        std::cerr << "Failed to open stream" << std::endl;
    }
    return 0;
}
