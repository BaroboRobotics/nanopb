#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include "oneof_visitors.pb.hpp"

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