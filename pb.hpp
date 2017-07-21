// Common supporting code for Nanopb-generated C++ code.

#ifndef PB_HPP_INCLUDED
#define PB_HPP_INCLUDED

#include "pb.h"

#include "pb_decode.h"
#include "pb_encode.h"

namespace nanopb {

template <class Struct>
constexpr const pb_field_t* field_array_ptr(const Struct&);
// Return a pointer to an array of fields completely describing the given message struct. This
// provides type safety to code which introspects the message structure (e.g., decode, encode).
// Specialized for every message struct type in generated code.

template <class Visitor, class Oneof>
bool visit(Visitor&& v, const Oneof& oneof);
// For oneofs whose possible values are each of distinct types, you can visit the value by calling
//
//   if (!visit(f, oneof)) { /* oneof contains an unrecognized type */ }
//
// with an `f` object which overloads its operator()(const T&) for every possible
// oneof value type. `util::overload()` can be useful in preparing such an `f` from lambdas, or you
// can implement a class with the appropriate overloads.

#define PB_define_decode_function(func) \
    template <class Struct> \
    bool func(pb_istream_t& input, Struct& dest_struct) { \
        return pb_##func(&input, field_array_ptr(dest_struct), &dest_struct); \
    }

#define PB_define_encode_function(func) \
    template <class Struct> \
    bool func(pb_ostream_t& output, const Struct& src_struct) { \
        return pb_##func(&output, field_array_ptr(src_struct), &src_struct); \
    }

PB_define_decode_function(decode)
PB_define_decode_function(decode_delimited)
PB_define_decode_function(decode_noinit)

PB_define_encode_function(encode)
PB_define_encode_function(encode_delimited)

#undef PB_define_encode_function
#undef PB_define_decode_function

// =======================================================================================
// Inline implementation

namespace _ {

template <class T>
struct visitor_impl;

}  // _

template <class Visitor, class Oneof>
bool visit(Visitor&& v, const Oneof& oneof) {
    return _::visitor_impl<Oneof>::apply(std::forward<Visitor>(v), oneof);
}

}  // nanopb

#endif
