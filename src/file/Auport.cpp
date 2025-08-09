#include "Audio.hpp"
#include "spdlog/spdlog.h"
#include <cstdint>
#include <cstring>
#include <file/Auport.hpp>

auFileReader::auFileReader(std::filesystem::path _path, AudioFileFormat _format) 
    : s_format(44100, 16, 2, auDtype::sInt) {
    path = _path;
    format = _format;
    file = std::ifstream(path, std::ios::binary);

    if(file.fail()){
        spdlog::error("Could not open \"{}\"!", path.string());
        error = true;
        return;
    }

    switch (format) {
        case AudioFileFormat::AudioFFWav:
            char buffer[4];
            file.read(buffer, 4);
            if(memcmp(buffer, "RIFF", 4)) {
                spdlog::error("\"{}\" is not a wav file(\"{}\" != \"RIFF\")!", path.string(), buffer);
                error = true;
                return;
            }
            uint32_t fsize;
            file.read(reinterpret_cast<char *>(&fsize), 4);
            file.read(buffer, 4);
            if(memcmp(buffer, "WAVE", 4)) {
                spdlog::error("\"{}\" is not a wav file(\"{}\" != \"WAVE\")!", path.string(), buffer);
                error = true;
                return;
            }
            file.read(buffer, 4);
            if(memcmp(buffer, "fmt ", 4)) {
                spdlog::error("\"{}\" is not a wav file(\"{}\" != \"fmt \")!", path.string(), buffer);
                error = true;
                return;
            }
            uint32_t fmt_size;
            file.read(reinterpret_cast<char *>(&fmt_size), 4);
            uint16_t fmt_type;
            file.read(reinterpret_cast<char *>(&fmt_type), 2);
            uint16_t num_channels;
            file.read(reinterpret_cast<char *>(&num_channels), 2);
            uint32_t sample_rate;
            file.read(reinterpret_cast<char *>(&sample_rate), 4);
            uint32_t bytes_per_sec;
            file.read(reinterpret_cast<char *>(&bytes_per_sec), 4);
            uint16_t block_size;
            file.read(reinterpret_cast<char *>(&block_size), 2);
            uint16_t bits_per_sample;
            file.read(reinterpret_cast<char *>(&bits_per_sample), 2);
            uint32_t dwSize;
            file.read(reinterpret_cast<char *>(&dwSize), 4);

            if(fmt_size != 16) {
                spdlog::error("\"{}\" is corrupted(invalid format_size)!", path.string());
                error = true;
                return;
            }

            if(bytes_per_sec != (sample_rate * num_channels * bits_per_sample) / 8) {
                spdlog::error("\"{}\" is corrupted(invalid bytes_per_sec)!", path.string());
                error = true;
                return;
            }

            if (block_size != (num_channels * bits_per_sample) / 8) {
                spdlog::error("\"{}\" is corrupted(invalid block_size)!", path.string());
                error = true;
                return;
            }
            s_format.sample_rate = sample_rate;
            s_format.bit_depth = bits_per_sample;
            s_format.channels = num_channels;
            switch (fmt_type) {
                case 1:
                    s_format.data_type = auDtype::sInt;
                    break;
                case 3:
                    s_format.data_type = auDtype::sFloat;
                    break;
                default:
                    spdlog::error("Unsupported wav format type {} in file \"{}\"!",fmt_type, path.string());
                    error = true;
                    return;
            }

            duration = fsize / bytes_per_sec;

            break;
        default:
            break;
    }
}
auFileReader::~auFileReader() {
    //theres really nothing to do here   
}
bool auFileReader::get_error() {
    return error;
}
uint32_t auFileReader::get_duration() {
    return duration;
}
auSFormat auFileReader::get_s_format() {
    return s_format;
}

bool auFileReader::read_chunk(char* buffer, size_t size) {
    file.read(buffer, size);
    return true;
}
auFileWriter::auFileWriter(std::filesystem::path path, AudioFileFormat format, auSFormat s_format) {
    
}
bool auFileWriter::get_error() {
    return error;
}
auFileWriter::~auFileWriter() {
    //theres really nothing to do here
}
bool auFileWriter::write_chunk(char* buffer, size_t size) {
    return true;
}