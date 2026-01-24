#pragma once

#include <functional>
#include <string_view>

#include <app/ImGuiLayer.h>
#include <core/Task.h>
#include <ui/AppState.h>

namespace WorkBalance::App::UI::Components {

/// @brief Self-contained component for rendering the task list panel
/// @details Handles task list display, task items, and add task functionality
class TaskListPanel {
  public:
    /// @brief Callbacks for task interactions
    struct Callbacks {
        std::function<void(std::string_view name, int estimated)> onTaskAdded;
        std::function<void(size_t index)> onTaskRemoved;
        std::function<void(size_t index, std::string_view name, int estimated, int completed)> onTaskUpdated;
        std::function<void(size_t index)> onTaskToggled;
    };

    /// @brief Constructs the task list panel component
    /// @param taskManager Reference to the task manager for reading/modifying tasks
    /// @param state Reference to the application state
    /// @param callbacks Callbacks to invoke on user interactions
    TaskListPanel(Core::TaskManager& taskManager, AppState& state, Callbacks callbacks);

    /// @brief Renders the complete task list panel with all tasks and add button
    void render();

  private:
    /// @brief Renders a single task item
    /// @param index The index of the task in the list
    /// @param task The task data to render
    void renderTaskItem(size_t index, const Core::Task& task);

    /// @brief Renders the "Add Task" button area
    void renderAddTaskButton();

    /// @brief Renders the Add Task popup dialog
    void renderAddTaskPopup();

    Core::TaskManager& m_task_manager;
    AppState& m_state;
    Callbacks m_callbacks;

    // Static buffer for new task name input
    char m_new_task_buffer[256] = "";
    int m_new_task_estimated = 1;
};

} // namespace WorkBalance::App::UI::Components
