#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include "oneof_visitors.pb.hpp"

#include "pb_encode.h"
#include "pb_decode.h"

#include <array>

struct TestVisitor {
    pb_size_t tag = 0;
    float value = 0.f;
    bool check(pb_size_t t, float v) const { return tag == t && value == v; }

    void operator()(foo_bar_One& one) {
        tag = foo_bar_MessageV1_one_tag;
        value = one.value;
    }

    void operator()(foo_bar_Two& two) {
        tag = foo_bar_MessageV1_two_tag;
        value = two.value;
    }

    void operator()(foo_bar_Three& three) {
        tag = foo_bar_MessageV2_three_tag;
        value = three.value;
    }
};

TEST_CASE("visit() calls visitors when appropriate") {
    TestVisitor visitor {};
    foo_bar_MessageV1 message {};

    REQUIRE(visitor.check(0, 0.f));  // visitor has not visited anything yet

    SUBCASE("visit() ignores nil field tag") {
        // Note that this case should be impossible to reproduce from `pb_decode`d data: Nanopb's
        // documentation says it treats a 0 field tag as an EOF in the stream.
        message.which_payload = 0;
        message.payload.one = {123.f};
        CHECK_FALSE(nanopb::visit(visitor, message.payload));
        CHECK(visitor.check(0, 0.f));  // still hasn't visited anything
    }

    SUBCASE("visit() ignores known but unbounded field tag") {
        message.which_payload = foo_bar_MessageV1_unbounded_tag;
        message.payload.one = {123.f};
        message.unbounded = {456.f};
        CHECK_FALSE(nanopb::visit(visitor, message.payload));
        CHECK(visitor.check(0, 0.f));  // still hasn't visited anything
    }

    SUBCASE("visit() ignores unknown field tag") {
        message.which_payload = 200;
        message.payload.one = {123.f};
        CHECK_FALSE(nanopb::visit(visitor, message.payload));
        CHECK(visitor.check(0, 0.f));  // still hasn't visited anything
    }

    SUBCASE("visit() calls visitor on first field") {
        message.which_payload = foo_bar_MessageV1_one_tag;
        message.payload.one = {123.f};
        CHECK(nanopb::visit(visitor, message.payload));
        CHECK(visitor.check(foo_bar_MessageV1_one_tag, 123.f));
    }

    SUBCASE("visit() calls visitor on second field") {
        message.which_payload = foo_bar_MessageV1_two_tag;
        message.payload.two = {456.f};
        CHECK(nanopb::visit(visitor, message.payload));
        CHECK(visitor.check(foo_bar_MessageV1_two_tag, 456.f));
    }
}

TEST_CASE("oneof schemas can be updated gracefully") {
    TestVisitor visitor {};
    std::array<pb_byte_t, 128> buf {};
    auto os = pb_ostream_from_buffer(buf.data(), buf.size());
    foo_bar_MessageV1 messageV1 {};
    foo_bar_MessageV2 messageV2 {};

    REQUIRE(visitor.check(0, 0.f));

    SUBCASE("message with obsolete field can be decoded by new clients") {
        messageV1.which_payload = foo_bar_MessageV1_one_tag;
        messageV1.payload.one = {123.f};
        REQUIRE(pb_encode(&os, foo_bar_MessageV1_fields, &messageV1));
        auto is = pb_istream_from_buffer(buf.data(), os.bytes_written);
        CHECK(pb_decode(&is, foo_bar_MessageV2_fields, &messageV2));
        CHECK_FALSE(nanopb::visit(visitor, messageV2.payload));
        CHECK(visitor.check(0, 0.f));
    }

    SUBCASE("message with new field can be decoded by old clients") {
        messageV2.which_payload = foo_bar_MessageV2_three_tag;
        messageV2.payload.three = {234.f};
        REQUIRE(pb_encode(&os, foo_bar_MessageV2_fields, &messageV2));
        auto is = pb_istream_from_buffer(buf.data(), os.bytes_written);
        CHECK(pb_decode(&is, foo_bar_MessageV1_fields, &messageV1));
        CHECK_FALSE(nanopb::visit(visitor, messageV1.payload));
        CHECK(visitor.check(0, 0.f));
    }

    SUBCASE("old message with compatible fields can be decoded and visited by new clients") {
        messageV1.which_payload = foo_bar_MessageV1_two_tag;
        messageV1.payload.one = {345.f};
        REQUIRE(pb_encode(&os, foo_bar_MessageV1_fields, &messageV1));
        auto is = pb_istream_from_buffer(buf.data(), os.bytes_written);
        CHECK(pb_decode(&is, foo_bar_MessageV2_fields, &messageV2));
        CHECK(nanopb::visit(visitor, messageV2.payload));
        CHECK(visitor.check(foo_bar_MessageV2_two_tag, 345.f));
    }

    SUBCASE("new message with compatible fields can be decoded and visited by old clients") {
        messageV2.which_payload = foo_bar_MessageV2_two_tag;
        messageV2.payload.two = {456.f};
        REQUIRE(pb_encode(&os, foo_bar_MessageV2_fields, &messageV2));
        auto is = pb_istream_from_buffer(buf.data(), os.bytes_written);
        CHECK(pb_decode(&is, foo_bar_MessageV1_fields, &messageV1));
        CHECK(nanopb::visit(visitor, messageV1.payload));
        CHECK(visitor.check(foo_bar_MessageV1_two_tag, 456.f));
    }
}
