#pragma once

#include <cstddef>
#include <cstdint>

enum auDtype {
    // invalid
    eInvalid  = -1,
    // signed integer
    sInt      = 0,
    // unsigned integer
    uInt      = 1,
    // floating point
    sFloat    = 2,
    // double floating point
    sDouble   = 3,
    // A-Law
    uALaw     = 4,
    // Mu-Law
    uMuLaw    = 5,
    // DVI ADPCM
    uDviAdpcm = 6,
    // Microsoft ADPCM
    uMsAdpcm  = 7
};

struct auSFormat {
    uint32_t sample_rate;
    uint32_t bit_depth;
    uint32_t channels;
    auDtype  data_type;
    auSFormat (uint32_t sr, uint32_t bd, uint32_t ch, auDtype dt) {
        sample_rate = sr;
        bit_depth   = bd;
        channels    = ch;
        data_type   = dt;
    }
    bool verify ();
};

size_t au_convert_buffer_size (auSFormat from, auSFormat to, size_t size);
bool   au_convert_buffer (auSFormat from, auSFormat to, char *from_buf,
                          size_t fromsize, char *to_buf);