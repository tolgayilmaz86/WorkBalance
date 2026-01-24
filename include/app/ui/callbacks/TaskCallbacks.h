#pragma once

#include <functional>
#include <string_view>

namespace WorkBalance::App::UI {

/// @brief Callbacks for task-related operations
struct TaskCallbacks {
    std::function<void(std::string_view name, int estimated)> onAdd;
    std::function<void(size_t index)> onRemove;
    std::function<void(size_t index, std::string_view name, int estimated, int completed)> onUpdate;
    std::function<void(size_t index)> onToggleCompletion;
};

} // namespace WorkBalance::App::UI
