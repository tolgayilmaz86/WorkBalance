#include <gtest/gtest.h>
#include "core/Timer.h"
#include "core/ITimeSource.h"
#include <memory>

using namespace WorkBalance::Core;
using namespace std::chrono_literals;

class TimerTest : public ::testing::Test {
  protected:
    void SetUp() override {
        mock_time_source = std::make_shared<MockTimeSource>();
        // Default durations: 25 min pomodoro, 5 min short break, 15 min long break
        timer = std::make_unique<Timer>(25 * 60, 5 * 60, 15 * 60, mock_time_source);
    }

    std::shared_ptr<MockTimeSource> mock_time_source;
    std::unique_ptr<Timer> timer;
};

TEST_F(TimerTest, InitialState) {
    EXPECT_EQ(timer->getState(), TimerState::Stopped);
    EXPECT_EQ(timer->getCurrentMode(), TimerMode::Pomodoro);
    EXPECT_EQ(timer->getRemainingTime(), 25 * 60);
}

TEST_F(TimerTest, StartSetsStateToRunning) {
    timer->start();
    EXPECT_EQ(timer->getState(), TimerState::Running);
    EXPECT_TRUE(timer->isRunning());
}

TEST_F(TimerTest, PauseSetsStateToPaused) {
    timer->start();
    timer->pause();
    EXPECT_EQ(timer->getState(), TimerState::Paused);
    EXPECT_TRUE(timer->isPaused());
}

TEST_F(TimerTest, StopSetsStateToStopped) {
    timer->start();
    mock_time_source->advance(60s);
    timer->update();
    timer->stop();

    EXPECT_EQ(timer->getState(), TimerState::Stopped);
    EXPECT_TRUE(timer->isStopped());
    // Note: stop() doesn't reset time, only changes state
    EXPECT_EQ(timer->getRemainingTime(), 25 * 60 - 60);
}

TEST_F(TimerTest, ToggleFromStopped) {
    timer->toggle();
    EXPECT_EQ(timer->getState(), TimerState::Running);
}

TEST_F(TimerTest, ToggleFromRunning) {
    timer->start();
    timer->toggle();
    EXPECT_EQ(timer->getState(), TimerState::Paused);
}

TEST_F(TimerTest, ToggleFromPaused) {
    timer->start();
    timer->pause();
    timer->toggle();
    EXPECT_EQ(timer->getState(), TimerState::Running);
}

TEST_F(TimerTest, UpdateDecrementsTime) {
    timer->start();
    mock_time_source->advance(10s);
    timer->update();

    EXPECT_EQ(timer->getRemainingTime(), 25 * 60 - 10);
}

TEST_F(TimerTest, UpdateWhilePausedDoesNotDecrementTime) {
    timer->start();
    mock_time_source->advance(10s);
    timer->update();
    int time_after_pause = timer->getRemainingTime();

    timer->pause();
    mock_time_source->advance(60s);
    timer->update();

    EXPECT_EQ(timer->getRemainingTime(), time_after_pause);
}

TEST_F(TimerTest, UpdateReturnsTrueWhenTimerCompletes) {
    timer->start();
    mock_time_source->advance(25min);

    bool completed = timer->update();

    EXPECT_TRUE(completed);
}

TEST_F(TimerTest, UpdateReturnsFalseWhenTimerStillRunning) {
    timer->start();
    mock_time_source->advance(10s);

    bool completed = timer->update();

    EXPECT_FALSE(completed);
}

TEST_F(TimerTest, SetModeChangesMode) {
    timer->setMode(TimerMode::ShortBreak);
    EXPECT_EQ(timer->getCurrentMode(), TimerMode::ShortBreak);
    EXPECT_EQ(timer->getRemainingTime(), 5 * 60);
}

TEST_F(TimerTest, SetModeToLongBreak) {
    timer->setMode(TimerMode::LongBreak);
    EXPECT_EQ(timer->getCurrentMode(), TimerMode::LongBreak);
    EXPECT_EQ(timer->getRemainingTime(), 15 * 60);
}

TEST_F(TimerTest, ResetResetsTimeButKeepsMode) {
    timer->setMode(TimerMode::ShortBreak);
    timer->start();
    mock_time_source->advance(60s);
    timer->update();

    timer->reset();

    EXPECT_EQ(timer->getCurrentMode(), TimerMode::ShortBreak);
    EXPECT_EQ(timer->getRemainingTime(), 5 * 60);
    // Note: reset() only resets time, doesn't change state
    EXPECT_EQ(timer->getState(), TimerState::Running);
}

TEST_F(TimerTest, SetDurationAffectsRemainingTimeIfNotRunning) {
    timer->setPomodoroDuration(30 * 60);
    EXPECT_EQ(timer->getRemainingTime(), 30 * 60);
}

TEST_F(TimerTest, TimerCompletesAtZero) {
    Timer short_timer(5, 3, 10, mock_time_source); // 5 second pomodoro
    short_timer.start();

    mock_time_source->advance(5s);
    bool completed = short_timer.update();

    EXPECT_TRUE(completed);
    EXPECT_LE(short_timer.getRemainingTime(), 0);
}

TEST_F(TimerTest, IsRunningIsFalseWhenStopped) {
    EXPECT_FALSE(timer->isRunning());
}

TEST_F(TimerTest, IsPausedIsFalseWhenStopped) {
    EXPECT_FALSE(timer->isPaused());
}

TEST_F(TimerTest, IsStoppedIsTrueInitially) {
    EXPECT_TRUE(timer->isStopped());
}
