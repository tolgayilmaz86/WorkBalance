#pragma once

#include <core/Task.h>

#include <functional>
#include <string_view>

namespace WorkBalance::Controllers {

/// @brief Pomodoro progress counters for tasks
struct PomodoroCounters {
    int target_pomodoros = 0;
    int completed_pomodoros = 0;
};

/// @brief Controller for managing task operations
class TaskController {
  public:
    /// @brief Constructs a TaskController
    /// @param manager Reference to the task manager to control
    explicit TaskController(Core::TaskManager& manager);

    /// @brief Add a new task
    void add(std::string_view name, int estimated_pomodoros);

    /// @brief Remove a task by index
    void remove(size_t index);

    /// @brief Update task details
    void update(size_t index, std::string_view name, int estimated_pomodoros, int completed_pomodoros);

    /// @brief Toggle task completion status
    void toggleCompletion(size_t index);

    /// @brief Increment completed pomodoros for a task
    void incrementPomodoros(size_t index);

    /// @brief Check if an index is valid
    [[nodiscard]] bool isValidIndex(size_t index) const noexcept;

    /// @brief Get the task manager
    [[nodiscard]] Core::TaskManager& getManager() noexcept {
        return m_manager;
    }
    [[nodiscard]] const Core::TaskManager& getManager() const noexcept {
        return m_manager;
    }

    /// @brief Get current pomodoro counters
    [[nodiscard]] PomodoroCounters getCounters() const;

    // Event callbacks
    std::function<void()> onTasksChanged;

  private:
    void notifyTasksChanged();

    Core::TaskManager& m_manager;
};

} // namespace WorkBalance::Controllers
