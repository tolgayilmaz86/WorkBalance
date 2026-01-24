#include <gtest/gtest.h>
#include "core/Event.h"

using namespace WorkBalance::Core;

class EventTest : public ::testing::Test {
  protected:
    VoidEvent void_event;
    IntEvent int_event;
    Event<std::string, int> string_int_event;
};

TEST_F(EventTest, SubscribeAndEmit) {
    bool was_called = false;
    void_event.subscribe([&was_called]() { was_called = true; });

    void_event.emit();

    EXPECT_TRUE(was_called);
}

TEST_F(EventTest, EmitWithNoSubscribers) {
    // Should not crash
    void_event.emit();
    SUCCEED();
}

TEST_F(EventTest, MultipleSubscribers) {
    int call_count = 0;

    void_event.subscribe([&call_count]() { ++call_count; });
    void_event.subscribe([&call_count]() { ++call_count; });
    void_event.subscribe([&call_count]() { ++call_count; });

    void_event.emit();

    EXPECT_EQ(call_count, 3);
}

TEST_F(EventTest, SubscribeReturnsUniqueHandlerId) {
    auto id1 = void_event.subscribe([]() {});
    auto id2 = void_event.subscribe([]() {});
    auto id3 = void_event.subscribe([]() {});

    EXPECT_NE(id1, id2);
    EXPECT_NE(id2, id3);
    EXPECT_NE(id1, id3);
}

TEST_F(EventTest, Unsubscribe) {
    int call_count = 0;

    auto id = void_event.subscribe([&call_count]() { ++call_count; });
    void_event.emit();
    EXPECT_EQ(call_count, 1);

    void_event.unsubscribe(id);
    void_event.emit();
    EXPECT_EQ(call_count, 1); // Should not increment after unsubscribe
}

TEST_F(EventTest, UnsubscribeInvalidId) {
    // Should not crash
    void_event.unsubscribe(99999);
    SUCCEED();
}

TEST_F(EventTest, IntEventPassesValue) {
    int received_value = 0;

    int_event.subscribe([&received_value](int value) { received_value = value; });

    int_event.emit(42);

    EXPECT_EQ(received_value, 42);
}

TEST_F(EventTest, MultipleArguments) {
    std::string received_str;
    int received_int = 0;

    string_int_event.subscribe([&](const std::string& s, int i) {
        received_str = s;
        received_int = i;
    });

    string_int_event.emit("hello", 123);

    EXPECT_EQ(received_str, "hello");
    EXPECT_EQ(received_int, 123);
}

TEST_F(EventTest, ClearAllSubscribers) {
    int call_count = 0;

    void_event.subscribe([&call_count]() { ++call_count; });
    void_event.subscribe([&call_count]() { ++call_count; });

    void_event.clear();
    void_event.emit();

    EXPECT_EQ(call_count, 0);
}

TEST_F(EventTest, EventGuardUnsubscribesOnDestruction) {
    int call_count = 0;

    {
        EventGuard<> guard(void_event, [&call_count]() { ++call_count; });
        void_event.emit();
        EXPECT_EQ(call_count, 1);
    } // guard goes out of scope

    void_event.emit();
    EXPECT_EQ(call_count, 1); // Should not increment after guard destroyed
}

TEST_F(EventTest, EventGuardMoveSemantics) {
    int call_count = 0;

    EventGuard<> guard1(void_event, [&call_count]() { ++call_count; });
    EventGuard<> guard2 = std::move(guard1);

    void_event.emit();
    EXPECT_EQ(call_count, 1);
}

TEST_F(EventTest, HasSubscribersReturnsCorrectly) {
    EXPECT_FALSE(void_event.hasSubscribers());

    auto id = void_event.subscribe([]() {});
    EXPECT_TRUE(void_event.hasSubscribers());

    void_event.unsubscribe(id);
    EXPECT_FALSE(void_event.hasSubscribers());
}
