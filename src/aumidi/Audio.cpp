#include "Audio.hpp"
#include <cstdint>
#include <spdlog/spdlog.h>

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
            spdlog::warn (
                "Invalid format: bit depth is not 4 for Microsoft ADPCM type!");
            return false;
        }
        break;
    default:
        spdlog::warn ("Invalid format: data type is invalid!");
        return false;
    }

    return true;
}

size_t au_convert_buffer_size (auSFormat from, auSFormat to, size_t size) {
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
    // basically just bit depth conversion(TODO)
    return true;
}

bool __uint_to_sint (auSFormat from, auSFormat to, char *from_buf,
                     size_t fromsize, char *to_buf) {
    // basically just bit depth conversion and sign extension(TODO)
    return true;
}

bool __uint_to_sfloat (auSFormat from, auSFormat to, char *from_buf,
                       size_t fromsize, char *to_buf) {
    // scaling(TODO)
    return true;
}

bool __uint_to_sdouble (auSFormat from, auSFormat to, char *from_buf,
                        size_t fromsize, char *to_buf) {
    // more scaling(TODO)
    return true;
}

bool __uint_to_ualaw (auSFormat from, auSFormat to, char *from_buf,
                      size_t fromsize, char *to_buf) {
    // h a r d(TODO)
    return true;
}

bool __uint_to_umulaw (auSFormat from, auSFormat to, char *from_buf,
                       size_t fromsize, char *to_buf) {
    // h a r d(TODO)
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
    if (!from.verify () || !to.verify ()) { return false; }
    return au_convert_call_table[(int32_t)from.data_type]
                                [(int32_t)to.data_type](from,to,from_buf, fromsize,
                                                        to_buf);
}