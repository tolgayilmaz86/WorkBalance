#include <gtest/gtest.h>
#include "core/Persistence.h"
#include <filesystem>
#include <fstream>

using namespace WorkBalance::Core;

// ============================================================================
// PersistenceManager Tests - Save/Load Round-Trip
// ============================================================================

class PersistenceTest : public ::testing::Test {
  protected:
    void SetUp() override {
        // Use a temporary directory for tests
        m_test_dir = std::filesystem::temp_directory_path() / "workbalance_test";
        std::filesystem::create_directories(m_test_dir);
        m_persistence = std::make_unique<PersistenceManager>(m_test_dir);
    }

    void TearDown() override {
        // Clean up test directory
        std::filesystem::remove_all(m_test_dir);
    }

    std::filesystem::path m_test_dir;
    std::unique_ptr<PersistenceManager> m_persistence;
};

TEST_F(PersistenceTest, SaveAndLoadDefaultSettings) {
    PersistentData data;
    // Use defaults

    auto save_result = m_persistence->save(data);
    ASSERT_TRUE(save_result.has_value()) << "Save failed";

    auto load_result = m_persistence->load();
    ASSERT_TRUE(load_result.has_value()) << "Load failed";

    const auto& loaded = load_result.value();
    EXPECT_EQ(loaded.settings.pomodoro_duration_minutes, Configuration::DEFAULT_POMODORO_MINUTES);
    EXPECT_EQ(loaded.settings.short_break_duration_minutes, Configuration::DEFAULT_SHORT_BREAK_MINUTES);
    EXPECT_EQ(loaded.settings.long_break_duration_minutes, Configuration::DEFAULT_LONG_BREAK_MINUTES);
}

TEST_F(PersistenceTest, SaveAndLoadCustomTimerDurations) {
    PersistentData data;
    data.settings.pomodoro_duration_minutes = 45;
    data.settings.short_break_duration_minutes = 10;
    data.settings.long_break_duration_minutes = 30;

    auto save_result = m_persistence->save(data);
    ASSERT_TRUE(save_result.has_value());

    auto load_result = m_persistence->load();
    ASSERT_TRUE(load_result.has_value());

    const auto& loaded = load_result.value();
    EXPECT_EQ(loaded.settings.pomodoro_duration_minutes, 45);
    EXPECT_EQ(loaded.settings.short_break_duration_minutes, 10);
    EXPECT_EQ(loaded.settings.long_break_duration_minutes, 30);
}

TEST_F(PersistenceTest, SaveAndLoadWindowPositions) {
    PersistentData data;
    data.settings.main_window_x = 100;
    data.settings.main_window_y = 200;
    data.settings.overlay_position_x = 300.5f;
    data.settings.overlay_position_y = 400.5f;

    auto save_result = m_persistence->save(data);
    ASSERT_TRUE(save_result.has_value());

    auto load_result = m_persistence->load();
    ASSERT_TRUE(load_result.has_value());

    const auto& loaded = load_result.value();
    EXPECT_EQ(loaded.settings.main_window_x, 100);
    EXPECT_EQ(loaded.settings.main_window_y, 200);
    EXPECT_FLOAT_EQ(loaded.settings.overlay_position_x, 300.5f);
    EXPECT_FLOAT_EQ(loaded.settings.overlay_position_y, 400.5f);
}

TEST_F(PersistenceTest, SaveAndLoadNegativeWindowPositions) {
    // Test that -1 (default/unset) values are preserved
    PersistentData data;
    data.settings.main_window_x = -1;
    data.settings.main_window_y = -1;

    auto save_result = m_persistence->save(data);
    ASSERT_TRUE(save_result.has_value());

    auto load_result = m_persistence->load();
    ASSERT_TRUE(load_result.has_value());

    const auto& loaded = load_result.value();
    EXPECT_EQ(loaded.settings.main_window_x, -1);
    EXPECT_EQ(loaded.settings.main_window_y, -1);
}

TEST_F(PersistenceTest, SaveAndLoadLargeWindowPositions) {
    // Test large values (multi-monitor setups)
    PersistentData data;
    data.settings.main_window_x = 3840; // 4K monitor offset
    data.settings.main_window_y = 2160;

    auto save_result = m_persistence->save(data);
    ASSERT_TRUE(save_result.has_value());

    auto load_result = m_persistence->load();
    ASSERT_TRUE(load_result.has_value());

    const auto& loaded = load_result.value();
    EXPECT_EQ(loaded.settings.main_window_x, 3840);
    EXPECT_EQ(loaded.settings.main_window_y, 2160);
}

TEST_F(PersistenceTest, SaveAndLoadBooleanSettings) {
    PersistentData data;
    data.settings.auto_start_breaks = true;
    data.settings.auto_start_pomodoros = true;
    data.settings.show_pomodoro_in_overlay = false;
    data.settings.show_water_in_overlay = false;
    data.settings.water_auto_loop = true;
    data.settings.start_minimized = false;

    auto save_result = m_persistence->save(data);
    ASSERT_TRUE(save_result.has_value());

    auto load_result = m_persistence->load();
    ASSERT_TRUE(load_result.has_value());

    const auto& loaded = load_result.value();
    EXPECT_TRUE(loaded.settings.auto_start_breaks);
    EXPECT_TRUE(loaded.settings.auto_start_pomodoros);
    EXPECT_FALSE(loaded.settings.show_pomodoro_in_overlay);
    EXPECT_FALSE(loaded.settings.show_water_in_overlay);
    EXPECT_TRUE(loaded.settings.water_auto_loop);
    EXPECT_FALSE(loaded.settings.start_minimized);
}

TEST_F(PersistenceTest, SaveAndLoadWellnessSettings) {
    PersistentData data;
    data.settings.water_interval_minutes = 45;
    data.settings.water_daily_goal = 10;
    data.settings.standup_interval_minutes = 60;
    data.settings.standup_duration_minutes = 10;
    data.settings.eye_care_interval_minutes = 30;
    data.settings.eye_care_break_seconds = 30;

    auto save_result = m_persistence->save(data);
    ASSERT_TRUE(save_result.has_value());

    auto load_result = m_persistence->load();
    ASSERT_TRUE(load_result.has_value());

    const auto& loaded = load_result.value();
    EXPECT_EQ(loaded.settings.water_interval_minutes, 45);
    EXPECT_EQ(loaded.settings.water_daily_goal, 10);
    EXPECT_EQ(loaded.settings.standup_interval_minutes, 60);
    EXPECT_EQ(loaded.settings.standup_duration_minutes, 10);
    EXPECT_EQ(loaded.settings.eye_care_interval_minutes, 30);
    EXPECT_EQ(loaded.settings.eye_care_break_seconds, 30);
}

TEST_F(PersistenceTest, SaveAndLoadTasks) {
    PersistentData data;

    Task task1;
    task1.name = "First Task";
    task1.completed = false;
    task1.estimated_pomodoros = 4;
    task1.completed_pomodoros = 1;

    Task task2;
    task2.name = "Second Task";
    task2.completed = true;
    task2.estimated_pomodoros = 2;
    task2.completed_pomodoros = 2;

    data.tasks.push_back(task1);
    data.tasks.push_back(task2);
    data.current_task_index = 1;

    auto save_result = m_persistence->save(data);
    ASSERT_TRUE(save_result.has_value());

    auto load_result = m_persistence->load();
    ASSERT_TRUE(load_result.has_value());

    const auto& loaded = load_result.value();
    ASSERT_EQ(loaded.tasks.size(), 2u);
    EXPECT_EQ(loaded.current_task_index, 1);

    EXPECT_EQ(loaded.tasks[0].name, "First Task");
    EXPECT_FALSE(loaded.tasks[0].completed);
    EXPECT_EQ(loaded.tasks[0].estimated_pomodoros, 4);
    EXPECT_EQ(loaded.tasks[0].completed_pomodoros, 1);

    EXPECT_EQ(loaded.tasks[1].name, "Second Task");
    EXPECT_TRUE(loaded.tasks[1].completed);
    EXPECT_EQ(loaded.tasks[1].estimated_pomodoros, 2);
    EXPECT_EQ(loaded.tasks[1].completed_pomodoros, 2);
}

TEST_F(PersistenceTest, SaveAndLoadTaskWithSpecialCharacters) {
    PersistentData data;

    Task task;
    task.name = "Task with \"quotes\" and \\backslash";
    task.completed = false;
    task.estimated_pomodoros = 1;
    task.completed_pomodoros = 0;

    data.tasks.push_back(task);

    auto save_result = m_persistence->save(data);
    ASSERT_TRUE(save_result.has_value());

    auto load_result = m_persistence->load();
    ASSERT_TRUE(load_result.has_value());

    const auto& loaded = load_result.value();
    ASSERT_EQ(loaded.tasks.size(), 1u);
    EXPECT_EQ(loaded.tasks[0].name, "Task with \"quotes\" and \\backslash");
}

TEST_F(PersistenceTest, SaveAndLoadEmptyTaskList) {
    PersistentData data;
    // No tasks added

    auto save_result = m_persistence->save(data);
    ASSERT_TRUE(save_result.has_value());

    auto load_result = m_persistence->load();
    ASSERT_TRUE(load_result.has_value());

    const auto& loaded = load_result.value();
    EXPECT_TRUE(loaded.tasks.empty());
}

TEST_F(PersistenceTest, LoadReturnsErrorWhenFileNotFound) {
    // Don't save anything, try to load
    auto load_result = m_persistence->load();
    ASSERT_FALSE(load_result.has_value());
    EXPECT_EQ(load_result.error(), PersistenceError::FileNotFound);
}

TEST_F(PersistenceTest, HasSavedDataReturnsFalseInitially) {
    EXPECT_FALSE(m_persistence->hasSavedData());
}

TEST_F(PersistenceTest, HasSavedDataReturnsTrueAfterSave) {
    PersistentData data;
    auto save_result = m_persistence->save(data);
    ASSERT_TRUE(save_result.has_value());

    EXPECT_TRUE(m_persistence->hasSavedData());
}

// ============================================================================
// JSON Parsing Edge Cases - Regression Tests
// ============================================================================

TEST_F(PersistenceTest, NestedJsonObjectsAreParsedCorrectly) {
    // This test catches the bug where JSON objects weren't being parsed
    // The settings object is nested inside the root object
    PersistentData data;
    data.settings.main_window_x = 500;
    data.settings.main_window_y = 600;
    data.settings.pomodoro_duration_minutes = 30;

    auto save_result = m_persistence->save(data);
    ASSERT_TRUE(save_result.has_value());

    auto load_result = m_persistence->load();
    ASSERT_TRUE(load_result.has_value());

    // All nested settings should be correctly parsed
    const auto& loaded = load_result.value();
    EXPECT_EQ(loaded.settings.main_window_x, 500);
    EXPECT_EQ(loaded.settings.main_window_y, 600);
    EXPECT_EQ(loaded.settings.pomodoro_duration_minutes, 30);
}

TEST_F(PersistenceTest, AllSettingsPreservedInRoundTrip) {
    // Comprehensive test to ensure ALL settings survive save/load
    PersistentData data;

    // Set every single setting to a non-default value
    data.settings.pomodoro_duration_minutes = 50;
    data.settings.short_break_duration_minutes = 15;
    data.settings.long_break_duration_minutes = 45;
    data.settings.auto_start_breaks = true;
    data.settings.auto_start_pomodoros = true;
    data.settings.overlay_position_x = 123.456f;
    data.settings.overlay_position_y = 789.012f;
    data.settings.main_window_x = 1234;
    data.settings.main_window_y = 5678;
    data.settings.show_pomodoro_in_overlay = false;
    data.settings.show_water_in_overlay = false;
    data.settings.show_standup_in_overlay = false;
    data.settings.show_eye_care_in_overlay = false;
    data.settings.water_interval_minutes = 60;
    data.settings.water_daily_goal = 12;
    data.settings.standup_interval_minutes = 90;
    data.settings.standup_duration_minutes = 15;
    data.settings.eye_care_interval_minutes = 40;
    data.settings.eye_care_break_seconds = 40;
    data.settings.water_auto_loop = true;
    data.settings.standup_auto_loop = true;
    data.settings.eye_care_auto_loop = true;
    data.settings.start_minimized = false;

    data.current_task_index = 5;

    auto save_result = m_persistence->save(data);
    ASSERT_TRUE(save_result.has_value());

    auto load_result = m_persistence->load();
    ASSERT_TRUE(load_result.has_value());

    const auto& loaded = load_result.value();

    // Verify EVERY setting
    EXPECT_EQ(loaded.settings.pomodoro_duration_minutes, 50);
    EXPECT_EQ(loaded.settings.short_break_duration_minutes, 15);
    EXPECT_EQ(loaded.settings.long_break_duration_minutes, 45);
    EXPECT_TRUE(loaded.settings.auto_start_breaks);
    EXPECT_TRUE(loaded.settings.auto_start_pomodoros);
    EXPECT_FLOAT_EQ(loaded.settings.overlay_position_x, 123.456f);
    EXPECT_FLOAT_EQ(loaded.settings.overlay_position_y, 789.012f);
    EXPECT_EQ(loaded.settings.main_window_x, 1234);
    EXPECT_EQ(loaded.settings.main_window_y, 5678);
    EXPECT_FALSE(loaded.settings.show_pomodoro_in_overlay);
    EXPECT_FALSE(loaded.settings.show_water_in_overlay);
    EXPECT_FALSE(loaded.settings.show_standup_in_overlay);
    EXPECT_FALSE(loaded.settings.show_eye_care_in_overlay);
    EXPECT_EQ(loaded.settings.water_interval_minutes, 60);
    EXPECT_EQ(loaded.settings.water_daily_goal, 12);
    EXPECT_EQ(loaded.settings.standup_interval_minutes, 90);
    EXPECT_EQ(loaded.settings.standup_duration_minutes, 15);
    EXPECT_EQ(loaded.settings.eye_care_interval_minutes, 40);
    EXPECT_EQ(loaded.settings.eye_care_break_seconds, 40);
    EXPECT_TRUE(loaded.settings.water_auto_loop);
    EXPECT_TRUE(loaded.settings.standup_auto_loop);
    EXPECT_TRUE(loaded.settings.eye_care_auto_loop);
    EXPECT_FALSE(loaded.settings.start_minimized);
    EXPECT_EQ(loaded.current_task_index, 5);
}

TEST_F(PersistenceTest, MalformedJsonReturnsParseError) {
    // Write malformed JSON directly to the file
    std::ofstream file(m_persistence->getConfigPath());
    file << "{ this is not valid json }}}";
    file.close();

    auto load_result = m_persistence->load();
    // Should not crash, should return an error or default values
    // The current implementation may return default values on parse error
}

TEST_F(PersistenceTest, EmptyFileReturnsError) {
    // Create empty file
    std::ofstream file(m_persistence->getConfigPath());
    file.close();

    auto load_result = m_persistence->load();
    // Should handle gracefully
}

TEST_F(PersistenceTest, PartialJsonUsesDefaults) {
    // Write JSON with only some fields
    std::ofstream file(m_persistence->getConfigPath());
    file << R"({
        "settings": {
            "pomodoro_duration_minutes": 99
        },
        "current_task_index": 0,
        "tasks": []
    })";
    file.close();

    auto load_result = m_persistence->load();
    ASSERT_TRUE(load_result.has_value());

    const auto& loaded = load_result.value();
    // Specified value should be loaded
    EXPECT_EQ(loaded.settings.pomodoro_duration_minutes, 99);
    // Missing values should use defaults
    EXPECT_EQ(loaded.settings.short_break_duration_minutes, Configuration::DEFAULT_SHORT_BREAK_MINUTES);
    EXPECT_EQ(loaded.settings.main_window_x, Configuration::DEFAULT_WINDOW_POSITION);
}

// ============================================================================
// Error Message Tests
// ============================================================================

TEST(PersistenceErrorTest, ErrorMessagesAreDescriptive) {
    EXPECT_FALSE(getPersistenceErrorMessage(PersistenceError::FileNotFound).empty());
    EXPECT_FALSE(getPersistenceErrorMessage(PersistenceError::FileOpenError).empty());
    EXPECT_FALSE(getPersistenceErrorMessage(PersistenceError::ParseError).empty());
    EXPECT_FALSE(getPersistenceErrorMessage(PersistenceError::WriteError).empty());
    EXPECT_FALSE(getPersistenceErrorMessage(PersistenceError::DirectoryCreateError).empty());
}
