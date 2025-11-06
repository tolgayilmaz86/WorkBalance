#pragma once

#include "../core/Timer.h"
#include "../core/Task.h"
#include "../../imgui.h"
#include "../../imgui_stdlib.h"
#include "../assets/fonts/IconsFontAwesome5Pro.h"

namespace WorkBalance::UI {
// Forward declarations
struct UIState;

/**
 * @brief Renders the timer mode selection buttons (Pomodoro, Short Break, Long Break)
 * @param timer The timer instance to check current mode
 * @param on_mode_change Callback when mode button is clicked
 */
void renderModeButtons(const Core::Timer& timer, std::function<void(Core::TimerMode)> on_mode_change);

/**
 * @brief Renders the main timer display with formatted time
 * @param remaining_time Current timer value in seconds
 */
void renderTimer(int remaining_time);

/**
 * @brief Renders the start/pause button with 3D effect
 * @param is_running Whether timer is currently running
 * @param on_toggle Callback when button is clicked
 */
void renderTimerControls(bool is_running, std::function<void()> on_toggle);

/**
 * @brief Renders the current active task name
 * @param tasks All tasks
 * @param current_task_index Index of current task
 */
void renderCurrentTask(std::span<const Core::Task> tasks, size_t current_task_index);

/**
 * @brief Renders the task list with checkboxes and edit buttons
 * @param tasks All tasks
 * @param current_task_index Currently selected task
 * @param background_color Current theme color
 * @param on_toggle_complete Callback when task checkbox clicked
 * @param on_edit Callback when edit button clicked
 * @param on_select Callback when task is selected as current
 */
void renderTaskList(std::span<const Core::Task> tasks, size_t current_task_index, const ImVec4& background_color,
                    std::function<void(size_t)> on_toggle_complete, std::function<void(size_t)> on_edit,
                    std::function<void(size_t)> on_select);

/**
 * @brief Renders the pomodoro counter display
 * @param completed_pomodoros Pomodoros completed
 * @param target_pomodoros Target pomodoros
 */
void renderPomodoroCounter(int completed_pomodoros, int target_pomodoros);

/**
 * @brief Renders header with settings, overlay, help, and shutdown buttons
 * @param on_settings Callback for settings button
 * @param on_overlay Callback for overlay toggle button
 * @param on_help Callback for help button
 * @param on_shutdown Callback for shutdown button
 */
void renderHeader(std::function<void()> on_settings, std::function<void()> on_overlay, std::function<void()> on_help,
                  std::function<void()> on_shutdown);

} // namespace WorkBalance::UI
