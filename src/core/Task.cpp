#include <core/Task.h>

#include <algorithm>

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
    if (index >= m_tasks.size()) {
        return;
    }

    Task& task = m_tasks[index];
    task.name = name;
    task.estimated_pomodoros = estimated;
    task.completed_pomodoros = completed;
    updateCounters();
}

void TaskManager::toggleTaskCompletion(size_t index) {
    if (index >= m_tasks.size()) {
        return;
    }

    m_tasks[index].completed = !m_tasks[index].completed;
    updateCounters();
}

void TaskManager::incrementTaskPomodoros(size_t index) {
    if (index >= m_tasks.size()) {
        return;
    }

    m_tasks[index].completed_pomodoros++;
    updateCounters();
}

std::vector<const Task*> TaskManager::getIncompleteTasks() const {
    std::vector<const Task*> incomplete;
    incomplete.reserve(m_tasks.size());

    for (const Task& task : m_tasks) {
        if (!task.completed) {
            incomplete.push_back(&task);
        }
    }

    return incomplete;
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
    m_target_pomodoros = 0;
    m_completed_pomodoros = 0;

    for (const Task& task : m_tasks) {
        if (!task.completed) {
            m_target_pomodoros += task.estimated_pomodoros;
        }
        m_completed_pomodoros += task.completed_pomodoros;
    }
}

} // namespace WorkBalance::Core
