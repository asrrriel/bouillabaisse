#include "Audio.hpp"
#include <spdlog/spdlog.h>

bool
auSFormat::verify () {

    if (sample_rate == 0) {
        spdlog::warn ("Invalid format: sample rate is 0!");
        return false;
    }
    if (bit_depth == 0) {
        spdlog::warn ("Invalid format: bit depth is 0!");
        return false;
    }
    if (channels == 0) {
        spdlog::warn ("Invalid format: channels is 0!");
        return false;
    }

    switch (data_type) {
    case auDtype::uInt:
    case auDtype::sInt:
        if (bit_depth != 8 && bit_depth != 16 && bit_depth != 24
            && bit_depth != 32 && bit_depth != 64) {
            spdlog::warn ("Invalid format: bit depth is not one of 8, 16, 24, "
                          "32, 64 for integer type!");
            return false;
        }
        break;
    case auDtype::sFloat:
        if (bit_depth != 32) {
            spdlog::warn (
                "Invalid format: bit depth is not 32 for float type!");
            return false;
        }
        break;
    case auDtype::sDouble:
        if (bit_depth != 64) {
            spdlog::warn (
                "Invalid format: bit depth is not 64 for double type!");
            return false;
        }
        break;
    case auDtype::uALaw:
        if (bit_depth != 8) {
            spdlog::warn (
                "Invalid format: bit depth is not 8 for A-Law type!");
            return false;
        }
        break;
    case auDtype::uMuLaw:
        if (bit_depth != 8) {
            spdlog::warn (
                "Invalid format: bit depth is not 8 for Mu-Law type!");
            return false;
        }
        break;
    default:
        spdlog::warn ("Invalid format: data type is invalid!");
        return false;
    }

    return true;
}