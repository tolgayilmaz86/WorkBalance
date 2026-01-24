#pragma once

#include "../../core/Configuration.h"

namespace WorkBalance {

/// @brief State for editing a task in the task edit popup
struct TaskEditState {
    int edit_index = -1;
    char name[Core::Configuration::MAX_TASK_NAME_LENGTH] = "";
    int estimated_pomodoros = Core::Configuration::DEFAULT_ESTIMATED_POMODOROS;
    int completed_pomodoros = Core::Configuration::DEFAULT_COMPLETED_POMODOROS;

    /// @brief Reset the edit state to defaults
    void reset() {
        edit_index = -1;
        name[0] = '\0';
        estimated_pomodoros = Core::Configuration::DEFAULT_ESTIMATED_POMODOROS;
        completed_pomodoros = Core::Configuration::DEFAULT_COMPLETED_POMODOROS;
    }
};

} // namespace WorkBalance
