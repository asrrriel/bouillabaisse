#include "Audio.hpp"
#include "io/Alsa.hpp"
#include "file/Auport.hpp"
#include <spdlog/spdlog.h>

#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <termios.h>
#include <unistd.h>

void
print_version () {
    spdlog::info ("Bouillabaisse version \"{}\"", VERSION);
}

int
main (int argc, char *argv[]) {
    print_version ();
    spdlog::info ("TODO: the whole thing :skull:");

    auDeviceManager adm;

    auto input_devices = adm.get_input_devices ();

    if (input_devices.empty ()) {
        spdlog::error ("No input audio devices found.");
        return 1;
    }

    for (const auto &device : input_devices) {
        spdlog::info ("Input Device: {} (Card: {}, Device: {})",
                      device.get_dev_name (), device.get_card_name (),
                      device.get_dev_number ());
    }

    auto output_devices = adm.get_output_devices ();

    if (output_devices.empty ()) {
        spdlog::error ("No output devices found!");
        return 1;
    }

    for (const auto &device : output_devices) {
        spdlog::info ("Output Device: {} (Card: {}, Device: {})",
                      device.get_dev_name (), device.get_card_name (),
                      device.get_dev_number ());
    }

    snd_pcm_t *input_handle  = nullptr;
    snd_pcm_t *output_handle = nullptr;

    if (input_devices.size () > 0) {
        auto &input_device = input_devices[0];
        if (input_device.open_stream (&input_handle, 44100, 2,
                                      SND_PCM_FORMAT_S16_LE)
            < 0) {
            spdlog::error ("Failed to open input stream.");
            return 1;
        }
    }

    if (output_devices.size () > 0) {
        auto &output_device = output_devices[0];
        if (output_device.open_stream (&output_handle, 44100, 2,
                                       SND_PCM_FORMAT_S16_LE)
            < 0) {
            spdlog::error ("Failed to open output stream.");
            return 1;
        }
    }

    auFileReader reader("test.wav", AudioFileFormat::AudioFFWav);

    if(reader.get_error()) {
        return 1;
    }

    auSFormat s_format = reader.get_s_format();

    spdlog::info("Audio file with sample rate {},bit depth {} channels {} read successfully", s_format.sample_rate, s_format.bit_depth, s_format.channels);

    spdlog::info("Audio file duration is {} seconds", reader.get_duration());
    return 0;
}
