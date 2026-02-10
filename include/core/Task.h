#pragma once

#include <cstddef>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace WorkBalance::Core {
struct Task {
    std::string name;
    bool completed = false;
    int estimated_pomodoros = 1;
    int completed_pomodoros = 0;

    [[nodiscard]] bool isComplete() const noexcept;

    [[nodiscard]] float getProgress() const noexcept;
};

class TaskManager {
  public:
    void addTask(std::string_view name, int estimated_pomodoros = 1);

    void removeTask(size_t index);

    void updateTask(size_t index, std::string_view name, int estimated, int completed);

    void toggleTaskCompletion(size_t index);

    void incrementTaskPomodoros(size_t index);

    void moveTask(size_t from_index, size_t to_index);

    [[nodiscard]] std::vector<const Task*> getIncompleteTasks() const;

    [[nodiscard]] std::span<const Task> getTasks() const noexcept;

    [[nodiscard]] std::span<Task> getTasks() noexcept;

    [[nodiscard]] const Task* getTask(size_t index) const noexcept;

    [[nodiscard]] Task* getTask(size_t index) noexcept;

    [[nodiscard]] size_t getTaskCount() const noexcept;
    [[nodiscard]] int getCompletedPomodoros() const noexcept;
    [[nodiscard]] int getTargetPomodoros() const noexcept;

    void clear() noexcept;

  private:
    void updateCounters() noexcept;

    std::vector<Task> m_tasks;
    int m_completed_pomodoros = 0;
    int m_target_pomodoros = 0;
};

} // namespace WorkBalance::Core
