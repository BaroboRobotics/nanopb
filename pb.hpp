// Common supporting code for Nanopb-generated C++ code.

#ifndef PB_HPP_INCLUDED
#define PB_HPP_INCLUDED

#include "pb.h"

#include "pb_decode.h"
#include "pb_encode.h"

namespace nanopb {

template <class Struct>
constexpr const pb_field_t* field_array_ptr();

#define PB_define_decode_function(func) \
    template <class Struct> \
    bool func(pb_istream_t& input, Struct& dest_struct) { \
        return pb_##func(&input, field_array_ptr<Struct>(), &dest_struct); \
    }

#define PB_define_encode_function(func) \
    template <class Struct> \
    bool func(pb_ostream_t& output, const Struct& src_struct) { \
        return pb_##func(&output, field_array_ptr<Struct>(), &src_struct); \
    }

PB_define_decode_function(decode)
PB_define_decode_function(decode_delimited)
PB_define_decode_function(decode_noinit)

PB_define_encode_function(encode)
PB_define_encode_function(encode_delimited)

#undef PB_define_encode_function
#undef PB_define_decode_function

}  // nanopb

#endif
