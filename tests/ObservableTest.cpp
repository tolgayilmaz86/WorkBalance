#include <gtest/gtest.h>
#include "core/Observable.h"
#include <string>
#include <vector>

using namespace WorkBalance::Core;

class ObservableTest : public ::testing::Test {
  protected:
    Observable<int> int_observable{0};
    Observable<std::string> string_observable{"initial"};
};

TEST_F(ObservableTest, InitialValueIsSet) {
    EXPECT_EQ(int_observable.get(), 0);
    EXPECT_EQ(string_observable.get(), "initial");
}

TEST_F(ObservableTest, ImplicitConversion) {
    int value = int_observable;
    EXPECT_EQ(value, 0);

    std::string str = string_observable;
    EXPECT_EQ(str, "initial");
}

TEST_F(ObservableTest, SetUpdatesValue) {
    int_observable.set(42);
    EXPECT_EQ(int_observable.get(), 42);
}

TEST_F(ObservableTest, SetNotifiesObserver) {
    int old_val = -1;
    int new_val = -1;

    int_observable.observe([&](int old_v, int new_v) {
        old_val = old_v;
        new_val = new_v;
    });

    int_observable.set(100);

    EXPECT_EQ(old_val, 0);
    EXPECT_EQ(new_val, 100);
}

TEST_F(ObservableTest, SetSameValueDoesNotNotify) {
    int notify_count = 0;

    int_observable.observe([&](int, int) { ++notify_count; });

    int_observable.set(0); // Same as initial value

    EXPECT_EQ(notify_count, 0);
}

TEST_F(ObservableTest, ForceSetAlwaysNotifies) {
    int notify_count = 0;

    int_observable.observe([&](int, int) { ++notify_count; });

    int_observable.forceSet(0); // Same value

    EXPECT_EQ(notify_count, 1);
}

TEST_F(ObservableTest, MultipleObservers) {
    int notify_count = 0;

    int_observable.observe([&](int, int) { ++notify_count; });
    int_observable.observe([&](int, int) { ++notify_count; });
    int_observable.observe([&](int, int) { ++notify_count; });

    int_observable.set(5);

    EXPECT_EQ(notify_count, 3);
}

TEST_F(ObservableTest, ClearObservers) {
    int notify_count = 0;

    int_observable.observe([&](int, int) { ++notify_count; });
    int_observable.observe([&](int, int) { ++notify_count; });

    int_observable.clearObservers();
    int_observable.set(10);

    EXPECT_EQ(notify_count, 0);
}

TEST_F(ObservableTest, HasObserversReturnsCorrectly) {
    EXPECT_FALSE(int_observable.hasObservers());

    int_observable.observe([](int, int) {});
    EXPECT_TRUE(int_observable.hasObservers());
}

TEST_F(ObservableTest, ObserverCount) {
    EXPECT_EQ(int_observable.observerCount(), 0u);

    int_observable.observe([](int, int) {});
    EXPECT_EQ(int_observable.observerCount(), 1u);

    int_observable.observe([](int, int) {});
    EXPECT_EQ(int_observable.observerCount(), 2u);
}

TEST_F(ObservableTest, ModifyUpdatesInPlace) {
    int_observable.set(10);

    bool changed = int_observable.modify([](int& val) { val += 5; });

    EXPECT_TRUE(changed);
    EXPECT_EQ(int_observable.get(), 15);
}

TEST_F(ObservableTest, ModifyReturnsFalseWhenNoChange) {
    int_observable.set(10);

    bool changed = int_observable.modify([](int& val) {
        val = 10; // No change
    });

    EXPECT_FALSE(changed);
}

TEST_F(ObservableTest, ModifyNotifiesOnChange) {
    int notify_count = 0;
    int_observable.set(10);

    int_observable.observe([&](int, int) { ++notify_count; });

    int_observable.modify([](int& val) { val = 20; });

    EXPECT_EQ(notify_count, 1);
}

TEST_F(ObservableTest, StringObservable) {
    std::string old_str, new_str;

    string_observable.observe([&](const std::string& old_v, const std::string& new_v) {
        old_str = old_v;
        new_str = new_v;
    });

    string_observable.set("updated");

    EXPECT_EQ(old_str, "initial");
    EXPECT_EQ(new_str, "updated");
}

// Tests for ComputedObservable
TEST(ComputedObservableTest, InitialComputation) {
    int base_value = 10;
    ComputedObservable<int> computed([&]() { return base_value * 2; });

    EXPECT_EQ(computed.get(), 20);
}

TEST(ComputedObservableTest, UpdateRecomputesValue) {
    int base_value = 10;
    ComputedObservable<int> computed([&]() { return base_value * 2; });

    base_value = 15;
    computed.update();

    EXPECT_EQ(computed.get(), 30);
}

TEST(ComputedObservableTest, UpdateNotifiesObserversOnChange) {
    int base_value = 10;
    int notify_count = 0;
    int old_val = -1;
    int new_val = -1;

    ComputedObservable<int> computed([&]() { return base_value * 2; });

    computed.observe([&](int old_v, int new_v) {
        ++notify_count;
        old_val = old_v;
        new_val = new_v;
    });

    base_value = 20;
    computed.update();

    EXPECT_EQ(notify_count, 1);
    EXPECT_EQ(old_val, 20);
    EXPECT_EQ(new_val, 40);
}

TEST(ComputedObservableTest, UpdateDoesNotNotifyWhenUnchanged) {
    int base_value = 10;
    int notify_count = 0;

    ComputedObservable<int> computed([&]() { return base_value * 2; });
    computed.observe([&](int, int) { ++notify_count; });

    computed.update(); // No change

    EXPECT_EQ(notify_count, 0);
}

TEST(ComputedObservableTest, ImplicitConversion) {
    ComputedObservable<int> computed([]() { return 42; });
    int value = computed;
    EXPECT_EQ(value, 42);
}
