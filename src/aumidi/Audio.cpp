#include "Audio.hpp"
#include <cmath>
#include <cstdint>
#include <spdlog/spdlog.h>
#include <sys/types.h>

bool auSFormat::verify () {

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
    case auDtype::uDviAdpcm:
        if (bit_depth != 4) {
            spdlog::warn (
                "Invalid format: bit depth is not 4 for DVI ADPCM type!");
            return false;
        }
        break;
    case auDtype::uMsAdpcm:
        if (bit_depth != 4) {
            spdlog::warn ("Invalid format: bit depth is not 4 for Microsoft "
                          "ADPCM type!");
            return false;
        }
        break;
    default:
        spdlog::warn ("Invalid format: data type is invalid!");
        return false;
    }

    return true;
}

int64_t __u_to_s (uint64_t u, uint8_t bit_depth) {
    return (int64_t)((u ^ (UINT64_C (1) << (bit_depth - 1)))
                     - (UINT64_C (1) << (bit_depth - 1)));
}

uint64_t __s_to_u (int64_t s, uint8_t bit_depth) {
    return (uint64_t)((s + (UINT64_C (1) << (bit_depth - 1)))
                      ^ (UINT64_C (1) << (bit_depth - 1)));
}

float __s_to_f (int64_t s, uint8_t bit_depth) {
    return (float)s / (float)(UINT64_C (1) << (bit_depth - 1));
}

int64_t __f_to_s (float f, uint8_t bit_depth) {
    return (int64_t)(f * (float)(UINT64_C (1) << (bit_depth - 1)));
}

uint8_t __u_to_ulaw (uint64_t u, uint8_t bit_depth) {
    const uint16_t BIAS = 0x84;

    int64_t u_s = __u_to_s (u, bit_depth);

    int64_t magnitude = std::abs (u_s);
    if (magnitude < BIAS) magnitude = BIAS;

    uint8_t segment = std::log2 (magnitude / BIAS);
    segment &= 0x7;

    uint8_t quantization = (magnitude >> (segment + 3)) & 0x0F;

    return ((segment << 4) | quantization) ^ ((u_s < 0) ? 0x7F : 0xFF);
}

uint64_t __ulaw_to_u (uint8_t ulaw, uint8_t bit_depth) {
    const uint16_t BIAS = 0x84;

    uint8_t real_bits = ulaw ^ 0xFF; // ulaw is inverted fsr

    bool    sign        = (real_bits & 0x80) >> 7;
    uint8_t segment     = (real_bits & 0x70) >> 4;
    uint8_t quanization = real_bits & 0x0F;
    printf ("sign: %u,segment: %u,quant: %u\n", sign, segment, quanization);

    uint16_t magnitude = ((quanization << 3) + BIAS) << segment;

    int16_t pcm_val = sign ? -magnitude : magnitude;

    return __s_to_u (pcm_val, bit_depth);
}

double __au_mix_float (double a, double b) { return (a + b) / 2; }

uint64_t __au_mix_uint (uint64_t a, uint64_t b, uint8_t bit_depth) {
    int64_t a_s   = __u_to_s (a, bit_depth);
    int64_t b_s   = __u_to_s (b, bit_depth);
    double  a_f   = __s_to_f (a_s, bit_depth);
    double  b_f   = __s_to_f (b_s, bit_depth);
    double  mixed = __au_mix_float (a_f, b_f);
    return __s_to_u (__f_to_s (mixed, bit_depth), bit_depth);
}

size_t au_convert_buffer_size (auSFormat from, auSFormat to, size_t size) {
    if (from.sample_rate != to.sample_rate) {
        spdlog::error ("unimplemented: upsampling/downsampling");
        return 0;
    }
    if (!from.verify () || !to.verify ()) { return 0; }
    return (size_t)(float (size) * (float (to.bit_depth) * float (to.channels))
                    / (float (from.bit_depth) * float (from.channels)));
}

// WARNING: the following call table is fucking huge(like my penis)
typedef bool (*au_convert_func) (auSFormat from, auSFormat to, char *from_buf,
                                 size_t fromsize, char *to_buf);

// From Uint
bool __uint_to_uint (auSFormat from, auSFormat to, char *from_buf,
                     size_t fromsize, char *to_buf) {
    size_t from_frame_size = (from.bit_depth / 8) * from.channels;
    size_t to_frame_size   = (to.bit_depth / 8) * to.channels;
    size_t from_num_frames = fromsize / from_frame_size;

    if (from.channels == to.channels) {
        for (size_t i = 0; i < from_num_frames; i++) {
            for (size_t j = 0; j < from.channels; j++) {
                uint64_t buffer = 0;
                memcpy (&buffer,
                        from_buf + (i * from_frame_size)
                            + (j * (from.bit_depth / 8)),
                        from.bit_depth / 8);
                if (to.bit_depth < from.bit_depth) {
                    buffer >>= (from.bit_depth
                                - to.bit_depth); // TODO: dithering isntead of
                                                 // clipping
                } else {
                    buffer <<= (to.bit_depth - from.bit_depth);
                }
                memcpy (to_buf + (i * to_frame_size)
                            + (j * (to.bit_depth / 8)),
                        &buffer, to.bit_depth / 8);
            }
        }
    } else if (from.channels > to.channels) {
        size_t group_size
            = size_t (ceil (from.channels / float (to.channels)));
        for (size_t i = 0; i < from_num_frames; i++) {
            size_t k = 0;
            for (size_t j = 0; j < to.channels; j++) {
                uint64_t buffer = 0, temp = 0;
                for (size_t l = 0; l < group_size; l++) {
                    memcpy (&temp,
                            from_buf + (i * from_frame_size)
                                + ((k + l) * (from.bit_depth / 8)),
                            from.bit_depth / 8);
                    buffer = __au_mix_uint (buffer, temp, from.bit_depth);
                }
                k += group_size;

                if (to.bit_depth < from.bit_depth) {
                    buffer >>= (from.bit_depth
                                - to.bit_depth); // TODO: dithering isntead of
                                                 // clipping
                } else {
                    buffer <<= (to.bit_depth - from.bit_depth);
                }
                memcpy (to_buf + (i * to_frame_size)
                            + (j * (to.bit_depth / 8)),
                        &buffer, to.bit_depth / 8);
            }
        }
    } else {
        for (size_t i = 0; i < from_num_frames; i++) {
            for (size_t j = 0; j < to.channels; j++) {
                if (j < from.channels) {
                    uint64_t buffer = 0;
                    memcpy (&buffer,
                            from_buf + (i * from_frame_size)
                                + (j * (from.bit_depth / 8)),
                            from.bit_depth / 8);
                    if (to.bit_depth < from.bit_depth) {
                        buffer >>= (from.bit_depth
                                    - to.bit_depth); // TODO: dithering isntead
                                                     // of clipping
                    } else {
                        buffer <<= (to.bit_depth - from.bit_depth);
                    }
                    memcpy (to_buf + (i * to_frame_size)
                                + (j * (to.bit_depth / 8)),
                            &buffer, to_frame_size);
                } else {
                    memset (to_buf + (i * to_frame_size)
                                + (j * (to.bit_depth / 8)),
                            0, to.bit_depth / 8);
                }
            }
        }
    }

    return true;
}

bool __uint_to_sint (auSFormat from, auSFormat to, char *from_buf,
                     size_t fromsize, char *to_buf) {
    size_t from_frame_size = (from.bit_depth / 8) * from.channels;
    size_t to_frame_size   = (to.bit_depth / 8) * to.channels;
    size_t from_num_frames = fromsize / from_frame_size;

    if (from.channels == to.channels) {
        for (size_t i = 0; i < from_num_frames; i++) {
            for (size_t j = 0; j < from.channels; j++) {
                uint64_t buffer = 0;
                memcpy (&buffer,
                        from_buf + (i * from_frame_size)
                            + (j * (from.bit_depth / 8)),
                        from.bit_depth / 8);
                if (to.bit_depth < from.bit_depth) {
                    buffer >>= (from.bit_depth
                                - to.bit_depth); // TODO: dithering isntead of
                                                 // clipping
                } else {
                    buffer <<= (to.bit_depth - from.bit_depth);
                }
                int64_t sbuffer = __u_to_s (buffer, from.bit_depth);
                memcpy (to_buf + (i * to_frame_size)
                            + (j * (to.bit_depth / 8)),
                        &sbuffer, to.bit_depth / 8);
            }
        }
    } else if (from.channels > to.channels) {
        size_t group_size
            = size_t (ceil (from.channels / float (to.channels)));
        for (size_t i = 0; i < from_num_frames; i++) {
            size_t k = 0;
            for (size_t j = 0; j < to.channels; j++) {
                uint64_t buffer = 0, temp = 0;
                for (size_t l = 0; l < group_size; l++) {
                    memcpy (&temp,
                            from_buf + (i * from_frame_size)
                                + ((k + l) * (from.bit_depth / 8)),
                            from.bit_depth / 8);
                    buffer = __au_mix_uint (buffer, temp, from.bit_depth);
                }
                k += group_size;

                if (to.bit_depth < from.bit_depth) {
                    buffer >>= (from.bit_depth
                                - to.bit_depth); // TODO: dithering isntead of
                                                 // clipping
                } else {
                    buffer <<= (to.bit_depth - from.bit_depth);
                }
                int64_t sbuffer = __u_to_s (buffer, from.bit_depth);
                memcpy (to_buf + (i * to_frame_size)
                            + (j * (to.bit_depth / 8)),
                        &sbuffer, to.bit_depth / 8);
            }
        }
    } else {
        for (size_t i = 0; i < from_num_frames; i++) {
            for (size_t j = 0; j < to.channels; j++) {
                if (j < from.channels) {
                    uint64_t buffer = 0;
                    memcpy (&buffer,
                            from_buf + (i * from_frame_size)
                                + (j * (from.bit_depth / 8)),
                            from.bit_depth / 8);
                    if (to.bit_depth < from.bit_depth) {
                        buffer >>= (from.bit_depth
                                    - to.bit_depth); // TODO: dithering isntead
                                                     // of clipping
                    } else {
                        buffer <<= (to.bit_depth - from.bit_depth);
                    }
                    int64_t sbuffer = __u_to_s (buffer, from.bit_depth);
                    memcpy (to_buf + (i * to_frame_size)
                                + (j * (to.bit_depth / 8)),
                            &sbuffer, to_frame_size);
                } else {
                    memset (to_buf + (i * to_frame_size)
                                + (j * (to.bit_depth / 8)),
                            0, to.bit_depth / 8);
                }
            }
        }
    }

    return true;
}

bool __uint_to_sfloat (auSFormat from, auSFormat to, char *from_buf,
                       size_t fromsize, char *to_buf) {
    size_t from_frame_size = (from.bit_depth / 8) * from.channels;
    size_t to_frame_size   = (to.bit_depth / 8) * to.channels;
    size_t from_num_frames = fromsize / from_frame_size;

    if (from.channels == to.channels) {
        for (size_t i = 0; i < from_num_frames; i++) {
            for (size_t j = 0; j < from.channels; j++) {
                uint64_t buffer = 0;
                memcpy (&buffer,
                        from_buf + (i * from_frame_size)
                            + (j * (from.bit_depth / 8)),
                        from.bit_depth / 8);
                if (to.bit_depth < from.bit_depth) {
                    buffer >>= (from.bit_depth
                                - to.bit_depth); // TODO: dithering isntead of
                                                 // clipping
                } else {
                    buffer <<= (to.bit_depth - from.bit_depth);
                }
                int64_t sbuffer = __u_to_s (buffer, from.bit_depth);
                float   fbuffer = __s_to_f (sbuffer, to.bit_depth);
                memcpy (to_buf + (i * to_frame_size)
                            + (j * (to.bit_depth / 8)),
                        &fbuffer, to.bit_depth / 8);
            }
        }
    } else if (from.channels > to.channels) {
        size_t group_size
            = size_t (ceil (from.channels / float (to.channels)));
        for (size_t i = 0; i < from_num_frames; i++) {
            size_t k = 0;
            for (size_t j = 0; j < to.channels; j++) {
                uint64_t buffer = 0, temp = 0;
                for (size_t l = 0; l < group_size; l++) {
                    memcpy (&temp,
                            from_buf + (i * from_frame_size)
                                + ((k + l) * (from.bit_depth / 8)),
                            from.bit_depth / 8);
                    buffer = __au_mix_uint (buffer, temp, from.bit_depth);
                }
                k += group_size;
                if (to.bit_depth < from.bit_depth) {
                    buffer >>= (from.bit_depth
                                - to.bit_depth); // TODO: dithering isntead of
                                                 // clipping
                } else {
                    buffer <<= (to.bit_depth - from.bit_depth);
                }
                int64_t sbuffer = __u_to_s (buffer, from.bit_depth);
                float   fbuffer = __s_to_f (sbuffer, to.bit_depth);
                // spdlog::info("buffer: {} sbuffer: {} fbuffer: {}", buffer,
                // sbuffer, fbuffer);
                memcpy (to_buf + (i * to_frame_size)
                            + (j * (to.bit_depth / 8)),
                        &fbuffer, to.bit_depth / 8);
            }
        }
    } else {
        for (size_t i = 0; i < from_num_frames; i++) {
            for (size_t j = 0; j < to.channels; j++) {
                if (j < from.channels) {
                    uint64_t buffer = 0;
                    memcpy (&buffer,
                            from_buf + (i * from_frame_size)
                                + (j * (from.bit_depth / 8)),
                            from.bit_depth / 8);
                    if (to.bit_depth < from.bit_depth) {
                        buffer >>= (from.bit_depth
                                    - to.bit_depth); // TODO: dithering isntead
                                                     // of clipping
                    } else {
                        buffer <<= (to.bit_depth - from.bit_depth);
                    }
                    int64_t sbuffer = __u_to_s (buffer, from.bit_depth);
                    float   fbuffer = __s_to_f (sbuffer, to.bit_depth);
                    memcpy (to_buf + (i * to_frame_size)
                                + (j * (to.bit_depth / 8)),
                            &fbuffer, to.bit_depth / 8);
                } else {
                    memset (to_buf + (i * to_frame_size)
                                + (j * (to.bit_depth / 8)),
                            0, to.bit_depth / 8);
                }
            }
        }
    }
    return true;
}

bool __uint_to_sdouble (auSFormat from, auSFormat to, char *from_buf,
                        size_t fromsize, char *to_buf) {
    size_t from_frame_size = (from.bit_depth / 8) * from.channels;
    size_t to_frame_size   = (to.bit_depth / 8) * to.channels;
    size_t from_num_frames = fromsize / from_frame_size;

    if (from.channels == to.channels) {
        for (size_t i = 0; i < from_num_frames; i++) {
            for (size_t j = 0; j < from.channels; j++) {
                uint64_t buffer = 0;
                memcpy (&buffer,
                        from_buf + (i * from_frame_size)
                            + (j * (from.bit_depth / 8)),
                        from.bit_depth / 8);
                if (to.bit_depth < from.bit_depth) {
                    buffer >>= (from.bit_depth
                                - to.bit_depth); // TODO: dithering isntead of
                                                 // clipping
                } else {
                    buffer <<= (to.bit_depth - from.bit_depth);
                }
                int64_t sbuffer = __u_to_s (buffer, from.bit_depth);
                double  fbuffer = __s_to_f (sbuffer, to.bit_depth);
                memcpy (to_buf + (i * to_frame_size)
                            + (j * (to.bit_depth / 8)),
                        &fbuffer, to.bit_depth / 8);
            }
        }
    } else if (from.channels > to.channels) {
        size_t group_size
            = size_t (ceil (from.channels / float (to.channels)));
        for (size_t i = 0; i < from_num_frames; i++) {
            size_t k = 0;
            for (size_t j = 0; j < to.channels; j++) {
                uint64_t buffer = 0, temp = 0;
                for (size_t l = 0; l < group_size; l++) {
                    memcpy (&temp,
                            from_buf + (i * from_frame_size)
                                + ((k + l) * (from.bit_depth / 8)),
                            from.bit_depth / 8);
                    buffer = __au_mix_uint (buffer, temp, from.bit_depth);
                }
                k += group_size;
                if (to.bit_depth < from.bit_depth) {
                    buffer >>= (from.bit_depth
                                - to.bit_depth); // TODO: dithering isntead of
                                                 // clipping
                } else {
                    buffer <<= (to.bit_depth - from.bit_depth);
                }
                int64_t sbuffer = __u_to_s (buffer, from.bit_depth);
                double  fbuffer = __s_to_f (sbuffer, to.bit_depth);
                // spdlog::info("buffer: {} sbuffer: {} fbuffer: {}", buffer,
                // sbuffer, fbuffer);
                memcpy (to_buf + (i * to_frame_size)
                            + (j * (to.bit_depth / 8)),
                        &fbuffer, to.bit_depth / 8);
            }
        }
    } else {
        for (size_t i = 0; i < from_num_frames; i++) {
            for (size_t j = 0; j < to.channels; j++) {
                if (j < from.channels) {
                    uint64_t buffer = 0;
                    memcpy (&buffer,
                            from_buf + (i * from_frame_size)
                                + (j * (from.bit_depth / 8)),
                            from.bit_depth / 8);
                    if (to.bit_depth < from.bit_depth) {
                        buffer >>= (from.bit_depth
                                    - to.bit_depth); // TODO: dithering isntead
                                                     // of clipping
                    } else {
                        buffer <<= (to.bit_depth - from.bit_depth);
                    }
                    int64_t sbuffer = __u_to_s (buffer, from.bit_depth);
                    double  fbuffer = __s_to_f (sbuffer, to.bit_depth);
                    memcpy (to_buf + (i * to_frame_size)
                                + (j * (to.bit_depth / 8)),
                            &fbuffer, to.bit_depth / 8);
                } else {
                    memset (to_buf + (i * to_frame_size)
                                + (j * (to.bit_depth / 8)),
                            0, to.bit_depth / 8);
                }
            }
        }
    }
    return true;
}

bool __uint_to_ualaw (auSFormat from, auSFormat to, char *from_buf,
                      size_t fromsize, char *to_buf) {
    // h a r d(TODO)
    return true;
}

bool __uint_to_umulaw (auSFormat from, auSFormat to, char *from_buf,
                       size_t fromsize, char *to_buf) {
    size_t from_frame_size = (from.bit_depth / 8) * from.channels;
    size_t to_frame_size   = (to.bit_depth / 8) * to.channels;
    size_t from_num_frames = fromsize / from_frame_size;

    if (from.channels == to.channels) {
        for (size_t i = 0; i < from_num_frames; i++) {
            for (size_t j = 0; j < from.channels; j++) {
                uint64_t buffer = 0;
                memcpy (&buffer,
                        from_buf + (i * from_frame_size)
                            + (j * (from.bit_depth / 8)),
                        from.bit_depth / 8);
                if (to.bit_depth < from.bit_depth) {
                    buffer >>= (from.bit_depth
                                - to.bit_depth); // TODO: dithering isntead of
                                                 // clipping
                } else {
                    buffer <<= (to.bit_depth - from.bit_depth);
                }
                uint8_t ulaw = __u_to_ulaw (buffer, from.bit_depth);
                memcpy (to_buf + (i * to_frame_size)
                            + (j * (to.bit_depth / 8)),
                        &ulaw, to.bit_depth / 8);
            }
        }
    } else if (from.channels > to.channels) {
        size_t group_size
            = size_t (ceil (from.channels / float (to.channels)));
        for (size_t i = 0; i < from_num_frames; i++) {
            size_t k = 0;
            for (size_t j = 0; j < to.channels; j++) {
                uint64_t buffer = 0, temp = 0;
                for (size_t l = 0; l < group_size; l++) {
                    memcpy (&temp,
                            from_buf + (i * from_frame_size)
                                + ((k + l) * (from.bit_depth / 8)),
                            from.bit_depth / 8);
                    buffer = __au_mix_uint (buffer, temp, from.bit_depth);
                }
                k += group_size;
                if (to.bit_depth < from.bit_depth) {
                    buffer >>= (from.bit_depth
                                - to.bit_depth); // TODO: dithering isntead of
                                                 // clipping
                } else {
                    buffer <<= (to.bit_depth - from.bit_depth);
                }
                uint8_t ulaw = __u_to_ulaw (buffer, from.bit_depth);
                memcpy (to_buf + (i * to_frame_size)
                            + (j * (to.bit_depth / 8)),
                        &ulaw, to.bit_depth / 8);
            }
        }
    } else {
        for (size_t i = 0; i < from_num_frames; i++) {
            for (size_t j = 0; j < to.channels; j++) {
                if (j < from.channels) {
                    uint64_t buffer = 0;
                    memcpy (&buffer,
                            from_buf + (i * from_frame_size)
                                + (j * (from.bit_depth / 8)),
                            from.bit_depth / 8);
                    if (to.bit_depth < from.bit_depth) {
                        buffer >>= (from.bit_depth
                                    - to.bit_depth); // TODO: dithering isntead
                                                     // of clipping
                    } else {
                        buffer <<= (to.bit_depth - from.bit_depth);
                    }
                    uint8_t ulaw = __u_to_ulaw (buffer, from.bit_depth);
                    memcpy (to_buf + (i * to_frame_size)
                                + (j * (to.bit_depth / 8)),
                            &ulaw, to.bit_depth / 8);
                } else {
                    memset (to_buf + (i * to_frame_size)
                                + (j * (to.bit_depth / 8)),
                            0, to.bit_depth / 8);
                }
            }
        }
    }
    return true;
}

bool __uint_to_udviadpcm (auSFormat from, auSFormat to, char *from_buf,
                          size_t fromsize, char *to_buf) {
    // h a r d(TODO)
    return true;
}

bool __uint_to_umsadpcm (auSFormat from, auSFormat to, char *from_buf,
                         size_t fromsize, char *to_buf) {
    // h a r d(TODO)
    return true;
}

// From Sint
bool __sint_to_uint (auSFormat from, auSFormat to, char *from_buf,
                     size_t fromsize, char *to_buf) {
    // basically just bit depth conversion and sign extension(TODO)
    return true;
}

bool __sint_to_sint (auSFormat from, auSFormat to, char *from_buf,
                     size_t fromsize, char *to_buf) {
    // basically just bit depth conversion(TODO)
    return true;
}

bool __sint_to_sfloat (auSFormat from, auSFormat to, char *from_buf,
                       size_t fromsize, char *to_buf) {
    // scaling(TODO)
    return true;
}

bool __sint_to_sdouble (auSFormat from, auSFormat to, char *from_buf,
                        size_t fromsize, char *to_buf) {
    // more scaling(TODO)
    return true;
}

bool __sint_to_ualaw (auSFormat from, auSFormat to, char *from_buf,
                      size_t fromsize, char *to_buf) {
    // h a r d(TODO)
    return true;
}

bool __sint_to_umulaw (auSFormat from, auSFormat to, char *from_buf,
                       size_t fromsize, char *to_buf) {
    // h a r d(TODO)
    return true;
}

bool __sint_to_udviadpcm (auSFormat from, auSFormat to, char *from_buf,
                          size_t fromsize, char *to_buf) {
    // h a r d(TODO)
    return true;
}

bool __sint_to_umsadpcm (auSFormat from, auSFormat to, char *from_buf,
                         size_t fromsize, char *to_buf) {
    // h a r d(TODO)
    return true;
}

// From Sfloat
bool __sfloat_to_sint (auSFormat from, auSFormat to, char *from_buf,
                       size_t fromsize, char *to_buf) {
    // scaling(TODO)
    return true;
}

bool __sfloat_to_uint (auSFormat from, auSFormat to, char *from_buf,
                       size_t fromsize, char *to_buf) {
    // scaling(TODO)
    return true;
}

bool __sfloat_to_sfloat (auSFormat from, auSFormat to, char *from_buf,
                         size_t fromsize, char *to_buf) {
    memcpy (to_buf, from_buf, fromsize);
    return true;
}

bool __sfloat_to_sdouble (auSFormat from, auSFormat to, char *from_buf,
                          size_t fromsize, char *to_buf) {
    // just bit depth conversion(TODO)
    return true;
}

bool __sfloat_to_ualaw (auSFormat from, auSFormat to, char *from_buf,
                        size_t fromsize, char *to_buf) {
    // h a r d(TODO)
    return true;
}

bool __sfloat_to_umulaw (auSFormat from, auSFormat to, char *from_buf,
                         size_t fromsize, char *to_buf) {
    // h a r d(TODO)
    return true;
}

bool __sfloat_to_udviadpcm (auSFormat from, auSFormat to, char *from_buf,
                            size_t fromsize, char *to_buf) {
    // h a r d(TODO)
    return true;
}

bool __sfloat_to_umsadpcm (auSFormat from, auSFormat to, char *from_buf,
                           size_t fromsize, char *to_buf) {
    // h a r d(TODO)
    return true;
}

// From Sdouble
bool __sdouble_to_sint (auSFormat from, auSFormat to, char *from_buf,
                        size_t fromsize, char *to_buf) {
    // more scaling(TODO)
    return true;
}

bool __sdouble_to_uint (auSFormat from, auSFormat to, char *from_buf,
                        size_t fromsize, char *to_buf) {
    // more scaling(TODO)
    return true;
}

bool __sdouble_to_sfloat (auSFormat from, auSFormat to, char *from_buf,
                          size_t fromsize, char *to_buf) {
    // just bit depth conversion(TODO)
    return true;
}

bool __sdouble_to_sdouble (auSFormat from, auSFormat to, char *from_buf,
                           size_t fromsize, char *to_buf) {
    memcpy (to_buf, from_buf, fromsize);
    return true;
}

bool __sdouble_to_ualaw (auSFormat from, auSFormat to, char *from_buf,
                         size_t fromsize, char *to_buf) {
    // h a r d(TODO)
    return true;
}

bool __sdouble_to_umulaw (auSFormat from, auSFormat to, char *from_buf,
                          size_t fromsize, char *to_buf) {
    // h a r d(TODO)
    return true;
}

bool __sdouble_to_udviadpcm (auSFormat from, auSFormat to, char *from_buf,
                             size_t fromsize, char *to_buf) {
    // h a r d(TODO)
    return true;
}

bool __sdouble_to_umsadpcm (auSFormat from, auSFormat to, char *from_buf,
                            size_t fromsize, char *to_buf) {
    // h a r d(TODO)
    return true;
}

// From UALaw
bool __ualaw_to_sint (auSFormat from, auSFormat to, char *from_buf,
                      size_t fromsize, char *to_buf) {
    // h a r d(TODO)
    return true;
}

bool __ualaw_to_uint (auSFormat from, auSFormat to, char *from_buf,
                      size_t fromsize, char *to_buf) {
    // h a r d(TODO)
    return true;
}

bool __ualaw_to_sfloat (auSFormat from, auSFormat to, char *from_buf,
                        size_t fromsize, char *to_buf) {
    // h a r d(TODO)
    return true;
}

bool __ualaw_to_sdouble (auSFormat from, auSFormat to, char *from_buf,
                         size_t fromsize, char *to_buf) {
    // h a r d(TODO)
    return true;
}

bool __ualaw_to_ualaw (auSFormat from, auSFormat to, char *from_buf,
                       size_t fromsize, char *to_buf) {
    memcpy (to_buf, from_buf, fromsize);
    return true;
}

bool __ualaw_to_umulaw (auSFormat from, auSFormat to, char *from_buf,
                        size_t fromsize, char *to_buf) {
    // h a r d but atleast no size change(TODO)
    return true;
}

bool __ualaw_to_udviadpcm (auSFormat from, auSFormat to, char *from_buf,
                           size_t fromsize, char *to_buf) {
    // h a r d(TODO)
    return true;
}

bool __ualaw_to_umsadpcm (auSFormat from, auSFormat to, char *from_buf,
                          size_t fromsize, char *to_buf) {
    // h a r d(TODO)
    return true;
}

// From UMuLaw
bool __umulaw_to_sint (auSFormat from, auSFormat to, char *from_buf,
                       size_t fromsize, char *to_buf) {
    // h a r d(TODO)
    return true;
}

bool __umulaw_to_uint (auSFormat from, auSFormat to, char *from_buf,
                       size_t fromsize, char *to_buf) {
    // h a r d(TODO)
    return true;
}

bool __umulaw_to_sfloat (auSFormat from, auSFormat to, char *from_buf,
                         size_t fromsize, char *to_buf) {
    // h a r d(TODO)
    return true;
}

bool __umulaw_to_sdouble (auSFormat from, auSFormat to, char *from_buf,
                          size_t fromsize, char *to_buf) {
    // h a r d(TODO)
    return true;
}

bool __umulaw_to_ualaw (auSFormat from, auSFormat to, char *from_buf,
                        size_t fromsize, char *to_buf) {
    // h a r d but atleast no size change(TODO)
    return true;
}

bool __umulaw_to_umulaw (auSFormat from, auSFormat to, char *from_buf,
                         size_t fromsize, char *to_buf) {
    memcpy (to_buf, from_buf, fromsize);
    return true;
}

bool __umulaw_to_udviadpcm (auSFormat from, auSFormat to, char *from_buf,
                            size_t fromsize, char *to_buf) {
    // h a r d(TODO)
    return true;
}

bool __umulaw_to_umsadpcm (auSFormat from, auSFormat to, char *from_buf,
                           size_t fromsize, char *to_buf) {
    // h a r d(TODO)
    return true;
}

// From uDviAdpcm
bool __udviadpcm_to_sint (auSFormat from, auSFormat to, char *from_buf,
                          size_t fromsize, char *to_buf) {
    // h a r d(TODO)
    return true;
}

bool __udviadpcm_to_uint (auSFormat from, auSFormat to, char *from_buf,
                          size_t fromsize, char *to_buf) {
    // h a r d(TODO)
    return true;
}

bool __udviadpcm_to_sfloat (auSFormat from, auSFormat to, char *from_buf,
                            size_t fromsize, char *to_buf) {
    // h a r d(TODO)
    return true;
}

bool __udviadpcm_to_sdouble (auSFormat from, auSFormat to, char *from_buf,
                             size_t fromsize, char *to_buf) {
    // h a r d(TODO)
    return true;
}

bool __udviadpcm_to_ualaw (auSFormat from, auSFormat to, char *from_buf,
                           size_t fromsize, char *to_buf) {
    // h a r d(TODO)
    return true;
}

bool __udviadpcm_to_umulaw (auSFormat from, auSFormat to, char *from_buf,
                            size_t fromsize, char *to_buf) {
    // h a r d(TODO)
    return true;
}

bool __udviadpcm_to_udviadpcm (auSFormat from, auSFormat to, char *from_buf,
                               size_t fromsize, char *to_buf) {
    memcpy (to_buf, from_buf, fromsize);
    return true;
}

bool __udviadpcm_to_umsadpcm (auSFormat from, auSFormat to, char *from_buf,
                              size_t fromsize, char *to_buf) {
    // h a r d but atleast no size change(TODO)
    return true;
}

// From uMsAdpcm
bool __umsadpcm_to_sint (auSFormat from, auSFormat to, char *from_buf,
                         size_t fromsize, char *to_buf) {
    // h a r d(TODO)
    return true;
}

bool __umsadpcm_to_uint (auSFormat from, auSFormat to, char *from_buf,
                         size_t fromsize, char *to_buf) {
    // h a r d(TODO)
    return true;
}

bool __umsadpcm_to_sfloat (auSFormat from, auSFormat to, char *from_buf,
                           size_t fromsize, char *to_buf) {
    // h a r d(TODO)
    return true;
}

bool __umsadpcm_to_sdouble (auSFormat from, auSFormat to, char *from_buf,
                            size_t fromsize, char *to_buf) {
    // h a r d(TODO)
    return true;
}

bool __umsadpcm_to_ualaw (auSFormat from, auSFormat to, char *from_buf,
                          size_t fromsize, char *to_buf) {
    // h a r d(TODO)
    return true;
}

bool __umsadpcm_to_umulaw (auSFormat from, auSFormat to, char *from_buf,
                           size_t fromsize, char *to_buf) {
    // h a r d(TODO)
    return true;
}

bool __umsadpcm_to_udviadpcm (auSFormat from, auSFormat to, char *from_buf,
                              size_t fromsize, char *to_buf) {
    // h a r d(TODO)
    return true;
}

bool __umsadpcm_to_umsadpcm (auSFormat from, auSFormat to, char *from_buf,
                             size_t fromsize, char *to_buf) {
    memcpy (to_buf, from_buf, fromsize);
    return true;
}

au_convert_func au_convert_call_table[8][8] = {
    {
     // from uInt
     __uint_to_sint,      // to sInt
      __uint_to_uint,      // to uInt
      __uint_to_sfloat,      // to sFloat
      __uint_to_sdouble,      // to sDouble
      __uint_to_ualaw,      // to uALaw
      __uint_to_umulaw,      // to uMuLaw
      __uint_to_udviadpcm,      // to uDviAdpcm
      __uint_to_umsadpcm   // to uMsAdpcm
     },
    {
     //      from sInt
     __sint_to_uint,      // to uInt
      __sint_to_sint,      // to sInt
      __sint_to_sfloat,      // to sFloat
      __sint_to_sdouble,      // to sDouble
      __sint_to_ualaw,      // to uALaw
      __sint_to_umulaw,      // to uMuLaw
      __sint_to_udviadpcm,      // to uDviAdpcm
      __sint_to_umsadpcm   // to uMsAdpcm
     },
    {
     //    from sFloat
     __sfloat_to_sint,    // to sInt
    __sfloat_to_uint,    // to uInt
    __sfloat_to_sfloat,    // to sFloat
    __sfloat_to_sdouble,    // to sDouble
    __sfloat_to_ualaw,    // to uALaw
    __sfloat_to_umulaw,    // to uMuLaw
    __sfloat_to_udviadpcm,    // to uDviAdpcm
    __sfloat_to_umsadpcm   // to uMsAdpcm
   },
    {
     //   from sDouble
     __sdouble_to_sint,   // to sInt
   __sdouble_to_uint,   // to uInt
   __sdouble_to_sfloat,   // to sFloat
   __sdouble_to_sdouble,   // to sDouble
   __sdouble_to_ualaw,   // to uALaw
   __sdouble_to_umulaw,   // to uMuLaw
   __sdouble_to_udviadpcm,   // to uDviAdpcm
   __sdouble_to_umsadpcm   // to uMsAdpcm
  },
    {
     //     from uALaw
     __ualaw_to_sint,     // to sInt
     __ualaw_to_uint,     // to uInt
     __ualaw_to_sfloat,     // to sFloat
     __ualaw_to_sdouble,     // to sDouble
     __ualaw_to_ualaw,     // to uALaw
     __ualaw_to_umulaw,     // to uMuLaw
     __ualaw_to_udviadpcm,     // to uDviAdpcm
     __ualaw_to_umsadpcm   // to uMsAdpcm
    },
    {
     //    from uMuLaw
     __umulaw_to_sint,    // to sInt
    __umulaw_to_uint,    // to uInt
    __umulaw_to_sfloat,    // to sFloat
    __umulaw_to_sdouble,    // to sDouble
    __umulaw_to_ualaw,    // to uALaw
    __umulaw_to_umulaw,    // to uMuLaw
    __umulaw_to_udviadpcm,    // to uDviAdpcm
    __umulaw_to_umsadpcm   // to uMsAdpcm
   },
    {
     // from uDviAdpcm
     __udviadpcm_to_sint, // to sInt
 __udviadpcm_to_uint, // to uInt
 __udviadpcm_to_sfloat, // to sFloat
 __udviadpcm_to_sdouble, // to sDouble
 __udviadpcm_to_ualaw, // to uALaw
 __udviadpcm_to_umulaw, // to uMuLaw
 __udviadpcm_to_udviadpcm, // to uDviAdpcm
 __udviadpcm_to_umsadpcm   // to uMsAdpcm
    },
    {
     //  from uMsAdpcm
     __umsadpcm_to_sint,  // to sInt
  __umsadpcm_to_uint,  // to uInt
  __umsadpcm_to_sfloat,  // to sFloat
  __umsadpcm_to_sdouble,  // to sDouble
  __umsadpcm_to_ualaw,  // to uALaw
  __umsadpcm_to_umulaw,  // to uMuLaw
  __umsadpcm_to_udviadpcm,  // to uDviAdpcm
  __umsadpcm_to_umsadpcm   // to uMsAdpcm
 }
};

bool au_convert_buffer (auSFormat from, auSFormat to, char *from_buf,
                        size_t fromsize, char *to_buf) {
    if (from.sample_rate != to.sample_rate) {
        spdlog::error ("unimplemented: upsampling/downsampling");
        return 0;
    }
    if (!from.verify () || !to.verify ()) { return false; }
    return au_convert_call_table[(int32_t)from.data_type]
                                [(int32_t)to.data_type](from, to, from_buf,
                                                        fromsize, to_buf);
}