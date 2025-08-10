#include "Audio.hpp"
#include <cstdint>
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

size_t au_convert_buffer_size(auSFormat from, auSFormat to, size_t size){
    if((int32_t)from.data_type < 0 || (int32_t)to.data_type < 0 || (int32_t)from.data_type > 5 || (int32_t)to.data_type > 5){
        return 0;
    }
    if(!from.verify() || !to.verify()){
        return 0;
    }
    return (size_t)(float(size) * float(to.bit_depth) / float(from.bit_depth));
}


//WARNING: the following call table is fucking huge(like my penis)
typedef bool (*au_convert_func)(char *from_buf, size_t fromsize,char *to_buf);

bool __uint_to_uint(char *from_buf, size_t fromsize,char *to_buf){
    //basically just bit depth conversion(TODO)
    return true;
}

bool __uint_to_sint(char *from_buf, size_t fromsize,char *to_buf){
    //basically just bit depth conversion and sign extension(TODO)
    return true;
}

bool __uint_to_sfloat(char *from_buf, size_t fromsize,char *to_buf){
    //scaling(TODO)
    return true;
}

bool __uint_to_sdouble(char *from_buf, size_t fromsize,char *to_buf){
    //more scaling(TODO)
    return true;
}

bool __uint_to_ualaw(char *from_buf, size_t fromsize,char *to_buf){
    //h a r d(TODO)
    return true;
}

bool __uint_to_umulaw(char *from_buf, size_t fromsize,char *to_buf){
    //h a r d(TODO)
    return true;
}

bool __sint_to_uint(char *from_buf, size_t fromsize,char *to_buf){
    //basically just bit depth conversion and sign extension(TODO)
    return true;
}

bool __sint_to_sint(char *from_buf, size_t fromsize,char *to_buf){
    //basically just bit depth conversion(TODO)
    return true;
}

bool __sint_to_sfloat(char *from_buf, size_t fromsize,char *to_buf){
    //scaling(TODO)
    return true;
}

bool __sint_to_sdouble(char *from_buf, size_t fromsize,char *to_buf){
    //more scaling(TODO)
    return true;
}

bool __sint_to_ualaw(char *from_buf, size_t fromsize,char *to_buf){
    //h a r d(TODO)
    return true;
}

bool __sint_to_umulaw(char *from_buf, size_t fromsize,char *to_buf){
    //h a r d(TODO)
    return true;
}

bool __sfloat_to_sint(char *from_buf, size_t fromsize,char *to_buf){
    //scaling(TODO)
    return true;
}

bool __sfloat_to_uint(char *from_buf, size_t fromsize,char *to_buf){
    //scaling(TODO)
    return true;
}

bool __sfloat_to_sfloat(char *from_buf, size_t fromsize,char *to_buf){
    //basically just a memcpy(TODO)
    return true;
}

bool __sfloat_to_sdouble(char *from_buf, size_t fromsize,char *to_buf){
    //just bit depth conversion(TODO)
    return true;
}

bool __sfloat_to_ualaw(char *from_buf, size_t fromsize,char *to_buf){
    //h a r d(TODO)
    return true;
}

bool __sfloat_to_umulaw(char *from_buf, size_t fromsize,char *to_buf){
    //h a r d(TODO)
    return true;
}

bool __sdouble_to_sint(char *from_buf, size_t fromsize,char *to_buf){
    //more scaling(TODO)
    return true;
}

bool __sdouble_to_uint(char *from_buf, size_t fromsize,char *to_buf){
    //more scaling(TODO)
    return true;
}

bool __sdouble_to_sfloat(char *from_buf, size_t fromsize,char *to_buf){
    //just bit depth conversion(TODO)
    return true;
}

bool __sdouble_to_sdouble(char *from_buf, size_t fromsize,char *to_buf){
    //basically just a memcpy(TODO)
    return true;
}

bool __sdouble_to_ualaw(char *from_buf, size_t fromsize,char *to_buf){
    //h a r d(TODO)
    return true;
}

bool __sdouble_to_umulaw(char *from_buf, size_t fromsize,char *to_buf){
    //h a r d(TODO)
    return true;
}

bool __ualaw_to_sint(char *from_buf, size_t fromsize,char *to_buf){
    //h a r d(TODO)
    return true;
}

bool __ualaw_to_uint(char *from_buf, size_t fromsize,char *to_buf){
    //h a r d(TODO)
    return true;
}

bool __ualaw_to_sfloat(char *from_buf, size_t fromsize,char *to_buf){
    //h a r d(TODO)
    return true;
}

bool __ualaw_to_sdouble(char *from_buf, size_t fromsize,char *to_buf){
    //h a r d(TODO)
    return true;
}

bool __ualaw_to_ualaw(char *from_buf, size_t fromsize,char *to_buf){
    //basically just a memcpy(TODO)
    return true;
}

bool __ualaw_to_umulaw(char *from_buf, size_t fromsize,char *to_buf){
    //h a r d but atleast no size change(TODO)
    return true;
}

bool __umulaw_to_sint(char *from_buf, size_t fromsize,char *to_buf){
    //h a r d(TODO)
    return true;
}

bool __umulaw_to_uint(char *from_buf, size_t fromsize,char *to_buf){
    //h a r d(TODO)
    return true;
}

bool __umulaw_to_sfloat(char *from_buf, size_t fromsize,char *to_buf){
    //h a r d(TODO)
    return true;
}

bool __umulaw_to_sdouble(char *from_buf, size_t fromsize,char *to_buf){
    //h a r d(TODO)
    return true;
}

bool __umulaw_to_ualaw(char *from_buf, size_t fromsize,char *to_buf){
    //h a r d but atleast no size change(TODO)
    return true;
}

bool __umulaw_to_umulaw(char *from_buf, size_t fromsize,char *to_buf){
    //basically just a memcpy(TODO)
    return true;
}

au_convert_func au_convert_call_table[6][6] = {
    { //from uInt
        __uint_to_sint,    //to sInt
        __uint_to_uint,    //to uInt
        __uint_to_sfloat,  //to sFloat
        __uint_to_sdouble, //to sDouble
        __uint_to_ualaw,   //to uALaw
        __uint_to_umulaw   //to uMuLaw
    },
    { //from sInt
        __sint_to_uint,    //to uInt
        __sint_to_sint,    //to sInt
        __sint_to_sfloat,  //to sFloat
        __sint_to_sdouble, //to sDouble
        __sint_to_ualaw,   //to uALaw
        __sint_to_umulaw   //to uMuLaw
    },
    { //from sFloat
        __sfloat_to_sint,    //to sInt
        __sfloat_to_uint,    //to uInt
        __sfloat_to_sfloat,  //to sFloat
        __sfloat_to_sdouble, //to sDouble
        __sfloat_to_ualaw,   //to uALaw
        __sfloat_to_umulaw   //to uMuLaw
    },
    { //from sDouble
        __sdouble_to_sint,    //to sInt
        __sdouble_to_uint,    //to uInt
        __sdouble_to_sfloat,  //to sFloat
        __sdouble_to_sdouble, //to sDouble
        __sdouble_to_ualaw,   //to uALaw
        __sdouble_to_umulaw   //to uMuLaw
    },
    { //from uALaw
        __ualaw_to_sint,    //to sInt
        __ualaw_to_uint,    //to uInt
        __ualaw_to_sfloat,  //to sFloat
        __ualaw_to_sdouble, //to sDouble
        __ualaw_to_ualaw,   //to uALaw
        __ualaw_to_umulaw   //to uMuLaw
    },
    { //from uMuLaw
        __umulaw_to_sint,    //to sInt
        __umulaw_to_uint,    //to uInt
        __umulaw_to_sfloat,  //to sFloat
        __umulaw_to_sdouble, //to sDouble
        __umulaw_to_ualaw,   //to uALaw
        __umulaw_to_umulaw   //to uMuLaw
    }
};

bool au_convert_buffer(auSFormat from, auSFormat to, char *from_buf, size_t fromsize,char *to_buf){
    if((int32_t)from.data_type < 0 || (int32_t)to.data_type < 0 || (int32_t)from.data_type > 5 || (int32_t)to.data_type > 5){
        return false;
    }
    return au_convert_call_table[(int32_t)from.data_type][(int32_t)to.data_type](from_buf,fromsize,to_buf);
}