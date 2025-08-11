#include "Audio.hpp"
#include "spdlog/spdlog.h"
#include <cstdint>
#include <cstring>
#include <file/Auport.hpp>
#include <sys/types.h>

auDtype fmt_type_to_dtype (uint16_t fmt_type, uint16_t bit_depth) {
    switch (fmt_type) {
    case 1:
        if (bit_depth == 8) { return auDtype::uInt; }
        return auDtype::sInt;
    case 2:
        return auDtype::uMsAdpcm;
    case 3:
        if (bit_depth == 64) { return auDtype::sDouble; }
        return auDtype::sFloat;
    case 6:
        return auDtype::uALaw;
    case 7:
        return auDtype::uMuLaw;
    case 17:     
        return auDtype::uDviAdpcm;
    case 0xFFFE: // TODO: support some extensible ones
    default:
        return auDtype::eInvalid; // invalid
    }
}

uint16_t dtype_to_fmt_type (auDtype dtype) {
    switch (dtype) {
    case auDtype::uInt:
    case auDtype::sInt:
        return 1;
    case auDtype::uMsAdpcm:
        return 2;
    case auDtype::sFloat:
        return 3;
    case auDtype::uALaw:
        return 6;
    case auDtype::uMuLaw:
        return 7;
    case auDtype::uDviAdpcm:
        return 17;
    default:
        return 0; // invalid
    }
}

auFileReader::auFileReader (std::filesystem::path _path,
                            AudioFileFormat       _format) :
    s_format (44100, 16, 2, auDtype::sInt) {
    path   = _path;
    format = _format;
    file   = std::ifstream (path, std::ios::binary);

    if (file.fail ()) {
        spdlog::error ("Could not open \"{}\"!", path.string ());
        error = true;
        return;
    }

    switch (format) {
    case AudioFileFormat::AudioFFWav:
        char buffer[5] = { 0 };
        file.read (buffer, 4);
        if (memcmp (buffer, "RIFF", 4)) {
            spdlog::error ("\"{}\" is not a wav file(\"{}\" != \"RIFF\")!",
                           path.string (), buffer);
            error = true;
            return;
        }
        uint32_t fsize;
        file.read (reinterpret_cast<char *> (&fsize), 4);
        file.read (buffer, 4);
        if (memcmp (buffer, "WAVE", 4)) {
            spdlog::error ("\"{}\" is not a wav file(\"{}\" != \"WAVE\")!",
                           path.string (), buffer);
            error = true;
            return;
        }
        file.read (buffer, 4);
        if (memcmp (buffer, "fmt ", 4)) {
            spdlog::error ("\"{}\" is not a wav file(\"{}\" != \"fmt \")!",
                           path.string (), buffer);
            error = true;
            return;
        }
        uint32_t fmt_size;
        file.read (reinterpret_cast<char *> (&fmt_size), 4);
        uint16_t fmt_type;
        file.read (reinterpret_cast<char *> (&fmt_type), 2);
        uint16_t num_channels;
        file.read (reinterpret_cast<char *> (&num_channels), 2);
        uint32_t sample_rate;
        file.read (reinterpret_cast<char *> (&sample_rate), 4);
        uint32_t bytes_per_sec;
        file.read (reinterpret_cast<char *> (&bytes_per_sec), 4);
        uint16_t block_size;
        file.read (reinterpret_cast<char *> (&block_size), 2);
        uint16_t bits_per_sample;
        file.read (reinterpret_cast<char *> (&bits_per_sample), 2);

        if (fmt_size != 16) {
            if (fmt_size < 16) {
                spdlog::error ("\"{}\" is corrupted(fmt_size is too small)!",
                               path.string ());
                error = true;
                return;
            }
            file.seekg (fmt_size - 16, std::ios::cur);
        }

        // this is not guaranteed for formats like ADPCM
        //if (bytes_per_sec
        //    != (sample_rate * num_channels * bits_per_sample) / 8) {
        //    spdlog::error ("\"{}\" is corrupted(invalid bytes_per_sec)!",
        //                   path.string ());
        //    error = true;
        //    return;
        //}
        //if (block_size != (num_channels * bits_per_sample) / 8) {
        //    spdlog::error ("\"{}\" is corrupted(invalid block_size)!",
        //                   path.string ());
        //    error = true;
        //    return;
        //}

        uint32_t data_size;
        bool     found = false;

        while (file.read (buffer, 4)
               && file.read (reinterpret_cast<char *> (&data_size), 4)) {
            if (std::memcmp (buffer, "data", 4) == 0) {
                found = true;
                break;
            }
            // Skip to next chunk (chunks are aligned to even sizes)
            file.seekg ((data_size + 1) & ~1, std::ios::cur);
        }

        if (!found) {
            spdlog::error ("\"{}\" is corrupted(missing data chunk)!",
                           path.string ());
            error = true;
            return;
        }
        s_format.sample_rate = sample_rate;
        s_format.bit_depth   = bits_per_sample;
        s_format.channels    = num_channels;
        s_format.data_type   = fmt_type_to_dtype (fmt_type, bits_per_sample);

        duration = data_size / bytes_per_sec;
        buf_size = data_size;

        break;
    }
}
auFileReader::~auFileReader () { file.close (); }
bool      auFileReader::get_error () { return error; }
uint32_t  auFileReader::get_duration () { return duration; }
uint32_t  auFileReader::get_buf_size () { return buf_size; }
auSFormat auFileReader::get_s_format () { return s_format; }

bool auFileReader::read_chunk (char *buffer, size_t size) {
    file.read (buffer, size);
    return true;
}
auFileWriter::auFileWriter (std::filesystem::path _path,
                            AudioFileFormat _format, auSFormat _s_format) :
    s_format (_s_format) {
    path   = _path;
    format = _format;
    file   = std::ofstream (path, std::ios::binary);

    auto write_u32 = [&] (uint32_t v) {
        file.write (reinterpret_cast<const char *> (&v), 4);
    };
    auto write_u16 = [&] (uint16_t v) {
        file.write (reinterpret_cast<const char *> (&v), 2);
    };
    auto write_str = [&] (const char *s, size_t len) { file.write (s, len); };

    switch (format) {
    case AudioFileFormat::AudioFFWav:
        write_str ("RIFF", 4);
        write_u32 (0);
        write_str ("WAVE", 4);
        write_str ("fmt ", 4);
        write_u32 (16);
        write_u16 (dtype_to_fmt_type (_s_format.data_type));
        write_u16 (_s_format.channels);
        write_u32 (_s_format.sample_rate);
        write_u32 (
            (_s_format.sample_rate * _s_format.channels * _s_format.bit_depth)
            / 8);
        write_u16 ((_s_format.channels * _s_format.bit_depth) / 8);
        write_u16 (_s_format.bit_depth);
        write_str ("data", 4);
        write_u32 (0);
        break;
    default:
        break;
    }
}
bool auFileWriter::get_error () { return error; }
auFileWriter::~auFileWriter () {
    uint32_t file_size = uint32_t (file.tellp ());

    uint32_t riff_size = file_size - 8; // file size - riff header
    file.seekp (4, std::ios::beg);
    file.write (reinterpret_cast<const char *> (&riff_size), 4);

    uint32_t data_size = file_size - 44; // file size - full header
    file.seekp (40, std::ios::beg);
    file.write (reinterpret_cast<const char *> (&data_size), 4);

    file.close ();
}
bool auFileWriter::write_chunk (char *buffer, size_t size) {
    file.write (buffer, size);
    return true;
}