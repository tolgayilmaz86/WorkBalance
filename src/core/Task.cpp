#include <core/Task.h>

#include <algorithm>
#include <numeric>
#include <ranges>

namespace WorkBalance::Core {

bool Task::isComplete() const noexcept {
    return completed || completed_pomodoros >= estimated_pomodoros;
}

float Task::getProgress() const noexcept {
    if (estimated_pomodoros == 0) {
        return 0.0f;
    }

    return static_cast<float>(completed_pomodoros) / static_cast<float>(estimated_pomodoros);
}

void TaskManager::addTask(std::string_view name, int estimated_pomodoros) {
    m_tasks.emplace_back(Task{std::string(name), false, estimated_pomodoros, 0});
    updateCounters();
}

void TaskManager::removeTask(size_t index) {
    if (index >= m_tasks.size()) {
        return;
    }

    m_tasks.erase(m_tasks.begin() + static_cast<std::vector<Task>::difference_type>(index));
    updateCounters();
}

void TaskManager::updateTask(size_t index, std::string_view name, int estimated, int completed) {
    if (Task* task = getTask(index); task != nullptr) {
        task->name = name;
        task->estimated_pomodoros = estimated;
        task->completed_pomodoros = completed;
        // Auto-mark as complete when pomodoros are achieved
        if (task->completed_pomodoros >= task->estimated_pomodoros) {
            task->completed = true;
        }
        updateCounters();
    }
}

void TaskManager::toggleTaskCompletion(size_t index) {
    if (Task* task = getTask(index); task != nullptr) {
        task->completed = !task->completed;
        updateCounters();
    }
}

void TaskManager::incrementTaskPomodoros(size_t index) {
    if (Task* task = getTask(index); task != nullptr) {
        task->completed_pomodoros++;
        // Auto-mark as complete when pomodoros are achieved
        if (task->completed_pomodoros >= task->estimated_pomodoros) {
            task->completed = true;
        }
        updateCounters();
    }
}

std::vector<const Task*> TaskManager::getIncompleteTasks() const {
    auto incomplete_view = m_tasks | std::views::filter([](const Task& t) { return !t.completed; }) |
                           std::views::transform([](const Task& t) { return &t; });

    return {incomplete_view.begin(), incomplete_view.end()};
}

void TaskManager::moveTask(size_t from_index, size_t to_index) {
    if (from_index >= m_tasks.size() || to_index >= m_tasks.size() || from_index == to_index) {
        return;
    }

    if (from_index < to_index) {
        // Moving forward: rotate [from, to+1) left by 1
        std::rotate(m_tasks.begin() + static_cast<std::vector<Task>::difference_type>(from_index),
                    m_tasks.begin() + static_cast<std::vector<Task>::difference_type>(from_index + 1),
                    m_tasks.begin() + static_cast<std::vector<Task>::difference_type>(to_index + 1));
    } else {
        // Moving backward: rotate [to, from+1) right by 1
        std::rotate(m_tasks.begin() + static_cast<std::vector<Task>::difference_type>(to_index),
                    m_tasks.begin() + static_cast<std::vector<Task>::difference_type>(from_index),
                    m_tasks.begin() + static_cast<std::vector<Task>::difference_type>(from_index + 1));
    }
    // No need to update counters as we're just reordering
}

std::span<const Task> TaskManager::getTasks() const noexcept {
    return {m_tasks.data(), m_tasks.size()};
}

std::span<Task> TaskManager::getTasks() noexcept {
    return {m_tasks.data(), m_tasks.size()};
}

const Task* TaskManager::getTask(size_t index) const noexcept {
    return index < m_tasks.size() ? &m_tasks[index] : nullptr;
}

Task* TaskManager::getTask(size_t index) noexcept {
    return index < m_tasks.size() ? &m_tasks[index] : nullptr;
}

size_t TaskManager::getTaskCount() const noexcept {
    return m_tasks.size();
}

int TaskManager::getCompletedPomodoros() const noexcept {
    return m_completed_pomodoros;
}

int TaskManager::getTargetPomodoros() const noexcept {
    return m_target_pomodoros;
}

void TaskManager::clear() noexcept {
    m_tasks.clear();
    updateCounters();
}

void TaskManager::updateCounters() noexcept {
    struct CounterTotals {
        int target = 0;
        int completed = 0;
    };

    const CounterTotals totals =
        std::ranges::fold_left(m_tasks, CounterTotals{}, [](CounterTotals acc, const Task& task) {
            if (!task.completed) {
                acc.target += task.estimated_pomodoros;
            }
            acc.completed += task.completed_pomodoros;
            return acc;
        });

    m_target_pomodoros = totals.target;
    m_completed_pomodoros = totals.completed;
}

} // namespace WorkBalance::Core
