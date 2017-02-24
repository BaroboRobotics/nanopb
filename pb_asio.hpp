// Boost.Asio/Nanopb interoperability code

#ifndef PB_ASIO_HPP_INCLUDED
#define PB_ASIO_HPP_INCLUDED

#include <boost/asio/buffer.hpp>

namespace nanopb {

template <class DynamicBuffer>
inline pb_ostream_t ostream_from_dynamic_buffer(DynamicBuffer& dynabuf) {
    // Create a pb_ostream_t that writes to an Asio/Networking TS/Beast DynamicBuffer.

    auto writer = [](pb_ostream_t* stream, const pb_byte_t* buf, size_t count) {
        auto& db = *static_cast<DynamicBuffer*>(stream->state);
        const auto b = boost::asio::buffer(buf, count);
        db.commit(boost::asio::buffer_copy(db.prepare(count), b));
        return true;
    };
    return pb_ostream_t{writer, &dynabuf, dynabuf.max_size() - dynabuf.size(), 0};
}

template <class DynamicBuffer>
inline pb_istream_t istream_from_dynamic_buffer(DynamicBuffer& dynabuf) {
    // Create a pb_istream_t that reads from an Asio/Networking TS/Beast DynamicBuffer.

    auto reader = [](pb_istream_t* stream, pb_byte_t* buf, size_t count) {
        auto& db = *static_cast<DynamicBuffer*>(stream->state);
        const auto b = boost::asio::buffer(buf, count);
        const auto n = boost::asio::buffer_copy(b, db.data());
        db.consume(n);
        return n == count;
    };
    return pb_istream_t{reader, &dynabuf, dynabuf.size()};
}

}  // nanopb

#endif
