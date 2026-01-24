#include <controllers/TaskController.h>

namespace WorkBalance::Controllers {

TaskController::TaskController(Core::TaskManager& manager) : m_manager(manager) {
}

void TaskController::add(std::string_view name, int estimated_pomodoros) {
    m_manager.addTask(name, estimated_pomodoros);
    notifyTasksChanged();
}

void TaskController::remove(size_t index) {
    if (!isValidIndex(index)) {
        return;
    }
    m_manager.removeTask(index);
    notifyTasksChanged();
}

void TaskController::update(size_t index, std::string_view name, int estimated_pomodoros, int completed_pomodoros) {
    if (!isValidIndex(index)) {
        return;
    }
    m_manager.updateTask(index, name, estimated_pomodoros, completed_pomodoros);
    notifyTasksChanged();
}

void TaskController::toggleCompletion(size_t index) {
    if (!isValidIndex(index)) {
        return;
    }
    m_manager.toggleTaskCompletion(index);
    notifyTasksChanged();
}

void TaskController::incrementPomodoros(size_t index) {
    if (!isValidIndex(index)) {
        return;
    }
    m_manager.incrementTaskPomodoros(index);
    notifyTasksChanged();
}

bool TaskController::isValidIndex(size_t index) const noexcept {
    return index < m_manager.getTasks().size();
}

PomodoroCounters TaskController::getCounters() const {
    return PomodoroCounters{.target_pomodoros = m_manager.getTargetPomodoros(),
                            .completed_pomodoros = m_manager.getCompletedPomodoros()};
}

void TaskController::notifyTasksChanged() {
    if (onTasksChanged) {
        onTasksChanged();
    }
}

} // namespace WorkBalance::Controllers
