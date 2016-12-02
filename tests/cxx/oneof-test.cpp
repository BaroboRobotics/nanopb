#include "doctest.h"

#include "oneof-test.pb.hpp"

#include "pb_encode.h"
#include "pb_decode.h"

#include <array>

struct TestVisitor {
    pb_size_t tag = 0;
    float value = 0.f;
    bool check(pb_size_t t, float v) const { return tag == t && value == v; }

    void operator()(const foo_bar_One& one) {
        tag = foo_bar_MessageV1_one_tag;
        value = one.value;
    }

    void operator()(const foo_bar_Two& two) {
        tag = foo_bar_MessageV1_two_tag;
        value = two.value;
    }

    void operator()(const foo_bar_Three& three) {
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
        message.which_arg = 0;
        message.arg.one = {123.f};
        CHECK_FALSE(nanopb::visit(visitor, message.arg));
        CHECK(visitor.check(0, 0.f));  // still hasn't visited anything
    }

    SUBCASE("visit() ignores known but unbounded field tag") {
        message.which_arg = foo_bar_MessageV1_unbounded_tag;
        message.arg.one = {123.f};
        message.unbounded = {456.f};
        CHECK_FALSE(nanopb::visit(visitor, message.arg));
        CHECK(visitor.check(0, 0.f));  // still hasn't visited anything
    }

    SUBCASE("visit() ignores unknown field tag") {
        message.which_arg = 200;
        message.arg.one = {123.f};
        CHECK_FALSE(nanopb::visit(visitor, message.arg));
        CHECK(visitor.check(0, 0.f));  // still hasn't visited anything
    }

    SUBCASE("visit() calls visitor on first field") {
        message.which_arg = foo_bar_MessageV1_one_tag;
        message.arg.one = {123.f};
        CHECK(nanopb::visit(visitor, message.arg));
        CHECK(visitor.check(foo_bar_MessageV1_one_tag, 123.f));
    }

    SUBCASE("visit() calls visitor on second field") {
        message.which_arg = foo_bar_MessageV1_two_tag;
        message.arg.two = {456.f};
        CHECK(nanopb::visit(visitor, message.arg));
        CHECK(visitor.check(foo_bar_MessageV1_two_tag, 456.f));
    }
}

TEST_CASE("assign() works") {
    foo_bar_MessageV1 messageV1 {};

    nanopb::assign(messageV1.arg, foo_bar_One{123.f});
    CHECK(messageV1.which_arg == foo_bar_MessageV1_one_tag);
    CHECK(messageV1.arg.one.value == 123.f);

    nanopb::assign(messageV1.arg, foo_bar_Two{456.f});
    CHECK(messageV1.which_arg == foo_bar_MessageV1_two_tag);
    CHECK(messageV1.arg.two.value == 456.f);
}

TEST_CASE("removed and added oneof fields are decodeable by 'incompatible' clients") {
    TestVisitor visitor {};
    std::array<pb_byte_t, 128> buf {};
    auto os = pb_ostream_from_buffer(buf.data(), buf.size());
    foo_bar_MessageV1 messageV1 {};
    foo_bar_MessageV2 messageV2 {};

    REQUIRE(visitor.check(0, 0.f));

    SUBCASE("message with removed oneof field can be decoded by new clients") {
        messageV1.which_arg = foo_bar_MessageV1_one_tag;
        messageV1.arg.one = {123.f};
        REQUIRE(nanopb::encode(os, messageV1));
        auto is = pb_istream_from_buffer(buf.data(), os.bytes_written);
        CHECK(nanopb::decode(is, messageV2));
        CHECK_FALSE(nanopb::visit(visitor, messageV2.arg));
        CHECK(visitor.check(0, 0.f));
    }

    SUBCASE("message with added oneof field can be decoded by old clients") {
        messageV2.which_arg = foo_bar_MessageV2_three_tag;
        messageV2.arg.three = {234.f};
        REQUIRE(nanopb::encode(os, messageV2));
        auto is = pb_istream_from_buffer(buf.data(), os.bytes_written);
        CHECK(nanopb::decode(is, messageV1));
        CHECK_FALSE(nanopb::visit(visitor, messageV1.arg));
        CHECK(visitor.check(0, 0.f));
    }

    SUBCASE("old message with compatible fields can be decoded and visited by new clients") {
        messageV1.which_arg = foo_bar_MessageV1_two_tag;
        messageV1.arg.one = {345.f};
        REQUIRE(nanopb::encode(os, messageV1));
        auto is = pb_istream_from_buffer(buf.data(), os.bytes_written);
        CHECK(nanopb::decode(is, messageV2));
        CHECK(nanopb::visit(visitor, messageV2.arg));
        CHECK(visitor.check(foo_bar_MessageV2_two_tag, 345.f));
    }

    SUBCASE("new message with compatible fields can be decoded and visited by old clients") {
        messageV2.which_arg = foo_bar_MessageV2_two_tag;
        messageV2.arg.two = {456.f};
        REQUIRE(nanopb::encode(os, messageV2));
        auto is = pb_istream_from_buffer(buf.data(), os.bytes_written);
        CHECK(nanopb::decode(is, messageV1));
        CHECK(nanopb::visit(visitor, messageV1.arg));
        CHECK(visitor.check(foo_bar_MessageV1_two_tag, 456.f));
    }
}
