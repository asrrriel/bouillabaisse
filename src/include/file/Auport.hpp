#pragma once

#include <cstdint>
#include <fstream>
#include <string>
#include <filesystem>
#include <Audio.hpp>

enum AudioFileFormat {
    AudioFFWav = 0
};

class auFileReader {
    bool error = false;
    std::filesystem::path path;
    std::ifstream file;
    AudioFileFormat format;
    auSFormat s_format;
    uint32_t duration;
    uint32_t buf_size;

    public:
    auFileReader(std::filesystem::path path, AudioFileFormat format);
    ~auFileReader();

    bool get_error();
    uint32_t get_duration();
    uint32_t get_buf_size();
    auSFormat get_s_format();
    bool read_chunk(char* buffer, size_t size);
};

class auFileWriter {
    bool error = false;
    std::filesystem::path path;
    std::ofstream file;
    AudioFileFormat format;
    auSFormat s_format;

    public:
    auFileWriter(std::filesystem::path path, AudioFileFormat format, auSFormat s_format);
    ~auFileWriter();

    bool get_error();
    bool write_chunk(char* buffer, size_t size);
};