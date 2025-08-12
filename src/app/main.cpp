#include "Audio.hpp"
#include "file/Auport.hpp"
#include "io/Alsa.hpp"
#include <spdlog/spdlog.h>

#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <termios.h>
#include <unistd.h>

void print_version () {
    spdlog::info ("Bouillabaisse version \"{}\"", VERSION);
}

int main (int argc, char *argv[]) {
    print_version ();

    auDeviceManager adm;

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

    auFileReader reader ("test.wav", AudioFileFormat::AudioFFWav);

    if (reader.get_error ()) { return 1; }

    auSFormat s_format = reader.get_s_format ();
    if (!s_format.verify ()) { return 1; }

    spdlog::info ("Audio file with sample rate {},bit depth {} channels {} "
                  "read successfully",
                  s_format.sample_rate, s_format.bit_depth, s_format.channels);

    spdlog::info ("Audio file duration is {} seconds", reader.get_duration ());

    uint32_t buffer_size = reader.get_buf_size ();
    uint32_t second_size
        = (s_format.sample_rate * s_format.channels * s_format.bit_depth) / 8;

    uint32_t data_read = 0;

    snd_pcm_t *speaker_handle = nullptr;

    auSFormat n_format
        = auSFormat (s_format.sample_rate, 8, 1, auDtype::uMuLaw);

    if (!n_format.verify ()) { return 1; }

    char *buffer = new char[buffer_size];

    reader.read_chunk (buffer, buffer_size);

    uint32_t new_buffer_size
        = au_convert_buffer_size (s_format, n_format, buffer_size);

    char *buffer2 = new char[new_buffer_size];

    au_convert_buffer (s_format, n_format, buffer, buffer_size, buffer2);

    auFileWriter writer ("test2.wav", AudioFileFormat::AudioFFWav, n_format);
    writer.write_chunk (buffer2, new_buffer_size);

    // if (output_devices.size () > 0) {
    //     auto &output_device = output_devices[0];
    //     if (output_device.open_stream (&speaker_handle, n_format) < 0) {
    //         spdlog::error ("Failed to open output stream.");
    //         return 1;
    //     }
    // }
    //
    // while (data_read < buffer_size) {
    //    output_devices[0].play_chunk (
    //        buffer + data_read,
    //        second_size / (s_format.channels * (s_format.bit_depth) / 8));
    //        spdlog::info ("Relayed {} seconds", data_read / second_size);
    //    data_read += second_size;
    //}
    //
    // output_devices[0].play_chunk (
    //    buffer + data_read, (buffer_size % second_size)
    //                / (s_format.channels * (s_format.bit_depth) / 8));

    delete[] buffer;
    delete[] buffer2;

    return 0;
}
