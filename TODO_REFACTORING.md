# WorkBalance Refactoring Plan

A comprehensive plan for improving code quality using SOLID principles, clean code practices, and C++23 modernization.

---

## Table of Contents

1. [SOLID Principles Violations](#1-solid-principles-violations)
2. [Clean Code Issues](#2-clean-code-issues)
3. [C++23 Modernization](#3-c23-modernization)
4. [Architecture Improvements](#4-architecture-improvements)
5. [Implementation Priority](#5-implementation-priority)

---

## 1. SOLID Principles Violations

### 1.1 Single Responsibility Principle (SRP)

> "A class should have only one reason to change."

#### Issue: `Application::Impl` is a God Class (744 lines)

**Location:** `src/app/Application.cpp`

**Current State:** Handles too many responsibilities:
- Timer management
- Wellness timer management
- Task management
- Persistence
- Audio
- UI coordination
- Window management
- System tray

**Before:**
```cpp
class Application::Impl {
    // Everything in one class
    void toggleTimer();
    void toggleWaterTimer();
    void toggleStandupTimer();
    void toggleEyeCareTimer();
    void addTask();
    void removeTask();
    void loadPersistedData();
    void savePersistedData();
    void playSound();
    void updateWindowTitle();
    void initializeSystemTray();
    // ... 50+ more methods
};
```

**After - Split into Controllers:**
```cpp
// include/controllers/TimerController.h
class TimerController {
public:
    explicit TimerController(Core::Timer& timer, IAudioService& audio);
    
    void toggle();
    void reset();
    void setMode(Core::TimerMode mode);
    
    // Events
    std::function<void(Core::TimerMode)> onModeChanged;
    std::function<void(int)> onTick;
    std::function<void()> onComplete;

private:
    Core::Timer& m_timer;
    IAudioService& m_audio;
};

// include/controllers/WellnessController.h
class WellnessController {
public:
    WellnessController(
        std::unique_ptr<Core::WellnessTimer> water,
        std::unique_ptr<Core::WellnessTimer> standup,
        std::unique_ptr<Core::WellnessTimer> eye_care
    );
    
    void toggleWater();
    void toggleStandup();
    void toggleEyeCare();
    void acknowledgeReminder(Core::WellnessType type);
    
private:
    std::unique_ptr<Core::WellnessTimer> m_water_timer;
    std::unique_ptr<Core::WellnessTimer> m_standup_timer;
    std::unique_ptr<Core::WellnessTimer> m_eye_care_timer;
};

// include/controllers/TaskController.h
class TaskController {
public:
    explicit TaskController(Core::TaskManager& manager);
    
    void add(std::string_view name, int estimated_pomodoros);
    void remove(size_t index);
    void update(size_t index, const Core::Task& task);
    void toggleCompletion(size_t index);
    
private:
    Core::TaskManager& m_manager;
};
```

---

#### Issue: `MainWindowView` is Too Large (1523 lines)

**Location:** `src/app/ui/MainWindowView.cpp`

**Recommendation:** Extract into focused UI components:

```cpp
// include/app/ui/components/SettingsPopup.h
class SettingsPopup {
public:
    struct Callbacks {
        std::function<void(int, int, int)> onPomodoroDurationsApplied;
        std::function<void(int, int, int, int, int, int)> onWellnessSettingsApplied;
    };
    
    SettingsPopup(AppState& state, Callbacks callbacks);
    void render();
    
private:
    void renderPomodoroSection();
    void renderWellnessSection();
    void renderOverlaySection();
    
    AppState& m_state;
    Callbacks m_callbacks;
};

// include/app/ui/components/TimerPanel.h
class TimerPanel {
public:
    TimerPanel(const Core::Timer& timer, AppState& state);
    void render();
    
private:
    void renderTimerDisplay();
    void renderControls();
    void renderModeSelector();
    
    const Core::Timer& m_timer;
    AppState& m_state;
};

// include/app/ui/components/TaskListPanel.h
class TaskListPanel {
public:
    struct Callbacks {
        std::function<void(std::string_view, int)> onTaskAdded;
        std::function<void(size_t)> onTaskRemoved;
        std::function<void(size_t)> onTaskToggled;
    };
    
    TaskListPanel(Core::TaskManager& tasks, AppState& state, Callbacks callbacks);
    void render();
    
private:
    void renderTaskInput();
    void renderTaskList();
    void renderTaskItem(size_t index, const Core::Task& task);
    
    Core::TaskManager& m_tasks;
    AppState& m_state;
    Callbacks m_callbacks;
};
```

---

#### Issue: `AppState` Holds Too Many Fields (40+)

**Location:** `include/ui/AppState.h`

**Before:**
```cpp
struct AppState {
    // UI state
    bool show_settings = false;
    bool show_help = false;
    bool main_window_overlay_mode = false;
    
    // Dragging state
    bool main_window_dragging = false;
    ImVec2 main_window_drag_offset{};
    
    // Timer settings (temporary for editing)
    int temp_pomodoro_duration = 25;
    int temp_short_break_duration = 5;
    
    // Wellness settings
    int temp_water_interval = 30;
    int water_count = 0;
    
    // Task state
    int current_task_index = 0;
    char edit_task_name[256] = "";
    
    // ... 30+ more fields
};
```

**After - Split into Focused Structs:**
```cpp
// include/ui/state/UIState.h
struct UIState {
    bool show_settings = false;
    bool show_help = false;
    bool main_window_overlay_mode = false;
    NavigationTab active_tab = NavigationTab::Pomodoro;
};

// include/ui/state/DragState.h
struct DragState {
    bool is_dragging = false;
    ImVec2 drag_offset{};
    ImVec2 position{};
};

struct WindowDragStates {
    DragState main_window;
    DragState main_overlay;
    DragState overlay;
};

// include/ui/state/SettingsEditState.h
struct PomodoroSettingsEdit {
    int pomodoro_duration = 25;
    int short_break_duration = 5;
    int long_break_duration = 15;
};

struct WellnessSettingsEdit {
    int water_interval = 30;
    int water_daily_goal = 8;
    int standup_interval = 45;
    int standup_duration = 5;
    int eye_interval = 20;
    int eye_break_duration = 20;
};

// include/ui/state/TaskEditState.h
struct TaskEditState {
    int edit_index = -1;
    std::string name;
    int estimated_pomodoros = 1;
    int completed_pomodoros = 0;
};

// include/ui/AppState.h - Composition
struct AppState {
    UIState ui;
    WindowDragStates drag;
    PomodoroSettingsEdit pomodoro_edit;
    WellnessSettingsEdit wellness_edit;
    TaskEditState task_edit;
    
    // Runtime counters (not editing state)
    int water_count = 0;
    int standup_count = 0;
    int pomodoro_count = 0;
    
    // Overlay visibility
    OverlayVisibility overlay_visibility;
};
```

---

### 1.2 Open/Closed Principle (OCP)

> "Software entities should be open for extension but closed for modification."

#### Issue: Timer Mode Handling Uses Switch Statements

**Location:** `src/core/Timer.cpp`, `src/app/ui/MainWindowView.cpp`

**Before:**
```cpp
int Timer::getCurrentDuration() const {
    switch (m_mode) {
        case TimerMode::Pomodoro:
            return m_pomodoro_duration;
        case TimerMode::ShortBreak:
            return m_short_break_duration;
        case TimerMode::LongBreak:
            return m_long_break_duration;
    }
    return m_pomodoro_duration;
}

// Repeated in multiple places for colors, icons, etc.
```

**After - Strategy Pattern with std::variant:**
```cpp
// include/core/TimerModeStrategy.h
struct PomodoroMode {
    static constexpr std::string_view name = "Pomodoro";
    static constexpr std::string_view icon = ICON_FA_CLOCK;
    int duration_seconds;
};

struct ShortBreakMode {
    static constexpr std::string_view name = "Short Break";
    static constexpr std::string_view icon = ICON_FA_COFFEE;
    int duration_seconds;
};

struct LongBreakMode {
    static constexpr std::string_view name = "Long Break";
    static constexpr std::string_view icon = ICON_FA_COFFEE;
    int duration_seconds;
};

using TimerMode = std::variant<PomodoroMode, ShortBreakMode, LongBreakMode>;

// Visitor for operations
struct GetDuration {
    int operator()(const PomodoroMode& m) const { return m.duration_seconds; }
    int operator()(const ShortBreakMode& m) const { return m.duration_seconds; }
    int operator()(const LongBreakMode& m) const { return m.duration_seconds; }
};

struct GetIcon {
    std::string_view operator()(const PomodoroMode&) const { return PomodoroMode::icon; }
    std::string_view operator()(const ShortBreakMode&) const { return ShortBreakMode::icon; }
    std::string_view operator()(const LongBreakMode&) const { return LongBreakMode::icon; }
};

// Usage
int duration = std::visit(GetDuration{}, timer.getMode());
```

---

### 1.3 Dependency Inversion Principle (DIP)

> "Depend on abstractions, not concretions."

#### Issue: Timer Uses `std::chrono::steady_clock` Directly

**Location:** `src/core/Timer.cpp`

**Problem:** Cannot unit test timer logic without waiting real time.

**Before:**
```cpp
void Timer::update() {
    if (!m_running) return;
    
    auto now = std::chrono::steady_clock::now();  // Direct dependency
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - m_last_update);
    // ...
}
```

**After - Inject Time Source:**
```cpp
// include/core/ITimeSource.h
class ITimeSource {
public:
    virtual ~ITimeSource() = default;
    virtual std::chrono::steady_clock::time_point now() const = 0;
};

// include/core/SystemTimeSource.h
class SystemTimeSource final : public ITimeSource {
public:
    std::chrono::steady_clock::time_point now() const override {
        return std::chrono::steady_clock::now();
    }
};

// For testing
class MockTimeSource final : public ITimeSource {
public:
    void advance(std::chrono::seconds duration) {
        m_current_time += duration;
    }
    
    std::chrono::steady_clock::time_point now() const override {
        return m_current_time;
    }

private:
    std::chrono::steady_clock::time_point m_current_time{};
};

// include/core/Timer.h
class Timer {
public:
    explicit Timer(
        int pomodoro_duration,
        int short_break_duration, 
        int long_break_duration,
        std::shared_ptr<ITimeSource> time_source = std::make_shared<SystemTimeSource>()
    );
    
private:
    std::shared_ptr<ITimeSource> m_time_source;
};

// Unit test example
TEST(TimerTest, CompletesAfterDuration) {
    auto mock_time = std::make_shared<MockTimeSource>();
    Timer timer(25 * 60, 5 * 60, 15 * 60, mock_time);
    
    timer.start();
    mock_time->advance(std::chrono::minutes(25));
    timer.update();
    
    EXPECT_TRUE(timer.isComplete());
}
```

---

#### Issue: `Application` Creates `PersistenceManager` Directly

**Before:**
```cpp
class Application::Impl {
    Core::PersistenceManager m_persistence;  // Concrete type
};
```

**After - Use Interface:**
```cpp
// include/core/IPersistenceService.h
class IPersistenceService {
public:
    virtual ~IPersistenceService() = default;
    
    [[nodiscard]] virtual std::expected<PersistentData, PersistenceError> load() const = 0;
    [[nodiscard]] virtual std::expected<void, PersistenceError> save(const PersistentData& data) const = 0;
    [[nodiscard]] virtual bool hasSavedData() const = 0;
};

// include/core/JsonPersistenceService.h
class JsonPersistenceService final : public IPersistenceService {
public:
    explicit JsonPersistenceService(std::filesystem::path config_path);
    
    std::expected<PersistentData, PersistenceError> load() const override;
    std::expected<void, PersistenceError> save(const PersistentData& data) const override;
    bool hasSavedData() const override;

private:
    std::filesystem::path m_config_path;
};

// Application uses interface
class Application::Impl {
    std::unique_ptr<IPersistenceService> m_persistence;
    
public:
    explicit Impl(std::unique_ptr<IPersistenceService> persistence = 
                  std::make_unique<JsonPersistenceService>(getDefaultConfigPath()));
};
```

---

### 1.4 Interface Segregation Principle (ISP)

> "Clients should not be forced to depend on interfaces they do not use."

#### Issue: `MainWindowCallbacks` Has Too Many Callbacks

**Location:** `include/app/ui/MainWindowView.h`

**Before:**
```cpp
struct MainWindowCallbacks {
    std::function<void()> onToggleTimer;
    std::function<void(Core::TimerMode)> onModeChange;
    std::function<void()> onToggleOverlayMode;
    std::function<void()> onRequestClose;
    std::function<void(int, int, int)> onDurationsApplied;
    std::function<void(int, int, int, int, int, int)> onWellnessSettingsApplied;
    std::function<void(std::string_view, int)> onTaskAdded;
    std::function<void(size_t)> onTaskRemoved;
    std::function<void(size_t, std::string_view, int, int)> onTaskUpdated;
    std::function<void(size_t)> onTaskCompletionToggled;
    std::function<void(NavigationTab)> onTabChanged;
    // More callbacks...
};
```

**After - Split by Domain:**
```cpp
// include/app/ui/callbacks/TimerCallbacks.h
struct TimerCallbacks {
    std::function<void()> onToggle;
    std::function<void(Core::TimerMode)> onModeChange;
    std::function<void()> onReset;
};

// include/app/ui/callbacks/TaskCallbacks.h
struct TaskCallbacks {
    std::function<void(std::string_view, int)> onAdd;
    std::function<void(size_t)> onRemove;
    std::function<void(size_t, std::string_view, int, int)> onUpdate;
    std::function<void(size_t)> onToggleCompletion;
};

// include/app/ui/callbacks/SettingsCallbacks.h
struct SettingsCallbacks {
    std::function<void(int, int, int)> onPomodoroDurationsApplied;
    std::function<void(int, int, int, int, int, int)> onWellnessSettingsApplied;
};

// include/app/ui/callbacks/WindowCallbacks.h
struct WindowCallbacks {
    std::function<void()> onToggleOverlayMode;
    std::function<void()> onRequestClose;
    std::function<void(NavigationTab)> onTabChanged;
};

// Compose in MainWindowView
class MainWindowView {
public:
    MainWindowView(
        // Dependencies...
        TimerCallbacks timer_callbacks,
        TaskCallbacks task_callbacks,
        SettingsCallbacks settings_callbacks,
        WindowCallbacks window_callbacks
    );
};
```

---

## 2. Clean Code Issues

### 2.1 Magic Numbers and Strings

#### Scattered Constants

**Locations:** Various files

**Before:**
```cpp
// In MainWindow.cpp
constexpr int overlay_width = 620;
constexpr int overlay_height = 70;
const int window_y = monitor.y + 10;

// In OverlayView.cpp
constexpr float padding_x = 40.0f;
constexpr float padding_y = 20.0f;

// In MainWindowView.cpp
ImGui::SetNextWindowSize(ImVec2(450, 500));
ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 12.0f);
```

**After - Centralize in Configuration:**
```cpp
// include/core/Configuration.h
namespace WorkBalance::Core::Configuration {

// Window dimensions
constexpr int DEFAULT_WINDOW_WIDTH = 400;
constexpr int DEFAULT_WINDOW_HEIGHT = 600;

// Overlay dimensions
namespace Overlay {
    constexpr int WIDTH = 620;
    constexpr int HEIGHT = 70;
    constexpr int TOP_MARGIN = 10;
    constexpr float PADDING_X = 40.0f;
    constexpr float PADDING_Y = 20.0f;
}

// UI styling
namespace Style {
    constexpr float WINDOW_ROUNDING = 12.0f;
    constexpr float BUTTON_ROUNDING = 4.0f;
    constexpr float FRAME_ROUNDING = 8.0f;
    
    // Settings popup
    constexpr float SETTINGS_WIDTH = 450.0f;
    constexpr float SETTINGS_HEIGHT = 500.0f;
    
    // Colors
    constexpr ImVec4 TEXT_WHITE{1.0f, 1.0f, 1.0f, 1.0f};
    constexpr ImVec4 TEXT_MUTED{0.7f, 0.7f, 0.7f, 1.0f};
}

// Timer defaults
namespace Timer {
    constexpr int DEFAULT_POMODORO_MINUTES = 25;
    constexpr int DEFAULT_SHORT_BREAK_MINUTES = 5;
    constexpr int DEFAULT_LONG_BREAK_MINUTES = 15;
    constexpr int POMODOROS_BEFORE_LONG_BREAK = 4;
}

// Wellness defaults
namespace Wellness {
    constexpr int DEFAULT_WATER_INTERVAL_MINUTES = 30;
    constexpr int DEFAULT_WATER_DAILY_GOAL = 8;
    constexpr int DEFAULT_STANDUP_INTERVAL_MINUTES = 45;
    constexpr int DEFAULT_STANDUP_DURATION_MINUTES = 5;
    constexpr int DEFAULT_EYE_INTERVAL_MINUTES = 20;
    constexpr int DEFAULT_EYE_BREAK_SECONDS = 20;
}

} // namespace WorkBalance::Core::Configuration
```

---

### 2.2 Code Duplication

#### JSON Parsing Helpers (~150 lines)

**Location:** `src/core/Persistence.cpp`

**Before:**
```cpp
std::string extractJsonValue(const std::string& json, const std::string& key) {
    const std::string search_key = "\"" + key + "\"";
    auto pos = json.find(search_key);
    // ... 40+ lines of parsing logic
}

int extractJsonInt(const std::string& json, const std::string& key, int default_value) {
    const std::string value = extractJsonValue(json, key);
    // ... parsing logic
}

float extractJsonFloat(const std::string& json, const std::string& key, float default_value) {
    // ... similar logic
}

bool extractJsonBool(const std::string& json, const std::string& key, bool default_value) {
    // ... similar logic
}
```

**Option A - Extract to Utility Class:**
```cpp
// include/core/JsonParser.h
class JsonParser {
public:
    explicit JsonParser(std::string_view json);
    
    [[nodiscard]] std::optional<std::string> getString(std::string_view key) const;
    [[nodiscard]] int getInt(std::string_view key, int default_value = 0) const;
    [[nodiscard]] float getFloat(std::string_view key, float default_value = 0.0f) const;
    [[nodiscard]] bool getBool(std::string_view key, bool default_value = false) const;
    [[nodiscard]] std::optional<JsonParser> getObject(std::string_view key) const;
    [[nodiscard]] std::vector<JsonParser> getArray(std::string_view key) const;

private:
    std::string_view m_json;
};

// Usage
JsonParser parser(json_content);
auto settings = parser.getObject("settings");
if (settings) {
    data.pomodoro_duration = settings->getInt("pomodoro_duration_minutes", 25);
    data.auto_start_breaks = settings->getBool("auto_start_breaks", false);
}
```

**Option B - Use nlohmann/json Library:**
```cpp
// Add to vcpkg.json
{
    "dependencies": ["nlohmann-json"]
}

// Usage becomes trivial
#include <nlohmann/json.hpp>

std::expected<PersistentData, PersistenceError> JsonPersistenceService::load() const {
    std::ifstream file(m_config_path);
    if (!file) {
        return std::unexpected(PersistenceError::FileNotFound);
    }
    
    try {
        auto json = nlohmann::json::parse(file);
        
        PersistentData data;
        data.settings.pomodoro_duration_minutes = json["settings"]["pomodoro_duration_minutes"];
        data.settings.auto_start_breaks = json["settings"]["auto_start_breaks"];
        // ... clean and simple
        
        return data;
    } catch (const nlohmann::json::exception& e) {
        return std::unexpected(PersistenceError::ParseError);
    }
}
```

---

### 2.3 C-Style Arrays

**Location:** `include/ui/AppState.h`

**Before:**
```cpp
struct AppState {
    char edit_task_name[256] = "";
    char new_task_name[256] = "";
};
```

**After - Use std::string:**
```cpp
struct TaskEditState {
    std::string name;
    std::string new_task_name;
};

// In UI code, use ImGui's std::string overloads
#include <imgui_stdlib.h>

ImGui::InputText("Task Name", &m_state.task_edit.name);
```

---

### 2.4 Long Functions

#### `renderSettingsPopup()` (~300 lines)

**Location:** `src/app/ui/MainWindowView.cpp`

**Recommendation:** Extract into methods per section:

```cpp
void MainWindowView::renderSettingsPopup() {
    if (!m_state.show_settings) return;
    
    ImGui::SetNextWindowSize(ImVec2(Config::Style::SETTINGS_WIDTH, Config::Style::SETTINGS_HEIGHT));
    
    if (ImGui::BeginPopupModal("Settings", &m_state.show_settings, ImGuiWindowFlags_NoResize)) {
        renderPomodoroSettings();
        ImGui::Separator();
        renderWellnessSettings();
        ImGui::Separator();
        renderOverlaySettings();
        ImGui::Separator();
        renderSettingsButtons();
        
        ImGui::EndPopup();
    }
}

void MainWindowView::renderPomodoroSettings() {
    ImGui::Text(ICON_FA_CLOCK " Pomodoro Settings");
    // ~30 lines for pomodoro section
}

void MainWindowView::renderWellnessSettings() {
    ImGui::Text(ICON_FA_HEART " Wellness Settings");
    renderWaterSettings();
    renderStandupSettings();
    renderEyeCareSettings();
}

void MainWindowView::renderWaterSettings() {
    // ~20 lines for water section
}
// ... etc
```

---

## 3. C++23 Modernization

### 3.1 Enable C++23 in CMake

**Location:** `CMakeLists.txt`

```cmake
# Change from
set(CMAKE_CXX_STANDARD 20)

# To
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
```

---

### 3.2 `std::expected` for Error Handling

**Location:** `include/core/Persistence.h`

**Before:**
```cpp
[[nodiscard]] std::optional<PersistentData> load() const;
// Caller doesn't know WHY it failed
```

**After:**
```cpp
#include <expected>

enum class PersistenceError {
    FileNotFound,
    PermissionDenied,
    ParseError,
    CorruptedData,
    UnknownError
};

[[nodiscard]] std::expected<PersistentData, PersistenceError> load() const;
[[nodiscard]] std::expected<void, PersistenceError> save(const PersistentData& data) const;

// Usage
auto result = persistence.load();
if (result) {
    applySettings(*result);
} else {
    switch (result.error()) {
        case PersistenceError::FileNotFound:
            // First run, use defaults
            break;
        case PersistenceError::ParseError:
            showError("Config file corrupted, using defaults");
            break;
        // ...
    }
}

// Or with monadic operations (C++23)
persistence.load()
    .transform([](auto& data) { applySettings(data); })
    .or_else([](auto error) { handleError(error); });
```

---

### 3.3 `std::format` and `std::print`

**Location:** `src/core/Persistence.cpp`

**Before:**
```cpp
std::ostringstream json;
json << "{\n";
json << "  \"settings\": {\n";
json << "    \"pomodoro_duration_minutes\": " << data.settings.pomodoro_duration_minutes << ",\n";
json << "    \"short_break_duration_minutes\": " << data.settings.short_break_duration_minutes << ",\n";
// ... tedious concatenation
```

**After:**
```cpp
#include <format>
#include <print>

std::string serializeToJson(const PersistentData& data) {
    return std::format(R"({{
  "settings": {{
    "pomodoro_duration_minutes": {},
    "short_break_duration_minutes": {},
    "long_break_duration_minutes": {},
    "auto_start_breaks": {},
    "auto_start_pomodoros": {},
    "overlay_position_x": {:.1f},
    "overlay_position_y": {:.1f},
    "main_window_x": {},
    "main_window_y": {}
  }},
  "current_task_index": {},
  "tasks": [{}]
}})",
        data.settings.pomodoro_duration_minutes,
        data.settings.short_break_duration_minutes,
        data.settings.long_break_duration_minutes,
        data.settings.auto_start_breaks,
        data.settings.auto_start_pomodoros,
        data.settings.overlay_position_x,
        data.settings.overlay_position_y,
        data.settings.main_window_x,
        data.settings.main_window_y,
        data.current_task_index,
        formatTasks(data.tasks)
    );
}

// For debugging/logging
void logTimerState(const Timer& timer) {
    std::println("Timer: mode={}, remaining={}s, running={}",
        std::to_underlying(timer.getMode()),
        timer.getRemainingTime(),
        timer.isRunning());
}
```

---

### 3.4 `std::ranges` Improvements

**Location:** Various files

**Before:**
```cpp
// Finding incomplete tasks
int incomplete_count = 0;
for (const auto& task : tasks) {
    if (!task.completed) {
        ++incomplete_count;
    }
}

// Filtering active timers
std::vector<Core::WellnessTimer*> active_timers;
if (m_water_timer && m_water_timer->isRunning()) {
    active_timers.push_back(m_water_timer.get());
}
if (m_standup_timer && m_standup_timer->isRunning()) {
    active_timers.push_back(m_standup_timer.get());
}
```

**After:**
```cpp
#include <ranges>

// Count incomplete tasks
auto incomplete_count = std::ranges::count_if(tasks, std::not_fn(&Task::completed));

// Or using projection
auto incomplete_count = std::ranges::count(tasks, false, &Task::completed);

// Filter active timers using ranges
auto timers = std::array{m_water_timer.get(), m_standup_timer.get(), m_eye_care_timer.get()};
auto active = timers 
    | std::views::filter([](auto* t) { return t && t->isRunning(); });

for (auto* timer : active) {
    // process active timer
}

// Transform and join for display
auto active_timer_strings = timers
    | std::views::filter([](auto* t) { return t && t->isRunning(); })
    | std::views::transform([](auto* t) { 
        return std::format("{} {}", getIcon(t->getType()), formatTime(t->getRemainingTime()));
    });
    
std::string display = active_timer_strings | std::views::join_with(" | "sv) | std::ranges::to<std::string>();
```

---

### 3.5 `std::flat_map` for Small Maps

**Location:** Theme/color mappings

**Before:**
```cpp
ImVec4 getColorForMode(TimerMode mode) {
    switch (mode) {
        case TimerMode::Pomodoro: return ImVec4(0.85f, 0.2f, 0.2f, 1.0f);
        case TimerMode::ShortBreak: return ImVec4(0.2f, 0.6f, 0.2f, 1.0f);
        case TimerMode::LongBreak: return ImVec4(0.2f, 0.4f, 0.8f, 1.0f);
    }
    return ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
}
```

**After:**
```cpp
#include <flat_map>

inline const std::flat_map<TimerMode, ImVec4> MODE_COLORS = {
    {TimerMode::Pomodoro,   {0.85f, 0.2f, 0.2f, 1.0f}},
    {TimerMode::ShortBreak, {0.2f, 0.6f, 0.2f, 1.0f}},
    {TimerMode::LongBreak,  {0.2f, 0.4f, 0.8f, 1.0f}},
};

inline const std::flat_map<TimerMode, std::string_view> MODE_ICONS = {
    {TimerMode::Pomodoro,   ICON_FA_CLOCK},
    {TimerMode::ShortBreak, ICON_FA_COFFEE},
    {TimerMode::LongBreak,  ICON_FA_COFFEE},
};

ImVec4 getColorForMode(TimerMode mode) {
    auto it = MODE_COLORS.find(mode);
    return it != MODE_COLORS.end() ? it->second : ImVec4{0.5f, 0.5f, 0.5f, 1.0f};
}
```

---

### 3.6 Deducing `this`

**Location:** Utility classes with method chaining

```cpp
// Builder pattern becomes cleaner
class TimerBuilder {
public:
    auto&& withPomodoroDuration(this auto&& self, int minutes) {
        self.m_pomodoro_duration = minutes * 60;
        return std::forward<decltype(self)>(self);
    }
    
    auto&& withShortBreakDuration(this auto&& self, int minutes) {
        self.m_short_break_duration = minutes * 60;
        return std::forward<decltype(self)>(self);
    }
    
    Timer build() && {
        return Timer(m_pomodoro_duration, m_short_break_duration, m_long_break_duration);
    }

private:
    int m_pomodoro_duration = 25 * 60;
    int m_short_break_duration = 5 * 60;
    int m_long_break_duration = 15 * 60;
};

// Usage
auto timer = TimerBuilder{}
    .withPomodoroDuration(30)
    .withShortBreakDuration(10)
    .build();
```

---

### 3.7 `constexpr` Improvements

**Location:** Configuration and utility functions

```cpp
// More functions can be constexpr in C++23
constexpr int minutesToSeconds(int minutes) noexcept {
    return minutes * 60;
}

constexpr int secondsToMinutes(int seconds) noexcept {
    return seconds / 60;
}

// consteval for compile-time only
consteval ImVec4 makeColor(float r, float g, float b, float a = 1.0f) {
    return ImVec4{r, g, b, a};
}

// if consteval for different compile/runtime behavior
std::string formatDuration(int seconds) {
    if consteval {
        // Compile-time: simple format
        return std::format("{}:{:02}", seconds / 60, seconds % 60);
    } else {
        // Runtime: might use locale, etc.
        return std::format("{}:{:02}", seconds / 60, seconds % 60);
    }
}
```

---

## 4. Architecture Improvements

### 4.1 Event System

Replace direct callbacks with a publish/subscribe event system for better decoupling:

```cpp
// include/core/Event.h
template<typename... Args>
class Event {
public:
    using Handler = std::function<void(Args...)>;
    using HandlerId = size_t;
    
    HandlerId subscribe(Handler handler) {
        auto id = m_next_id++;
        m_handlers[id] = std::move(handler);
        return id;
    }
    
    void unsubscribe(HandlerId id) {
        m_handlers.erase(id);
    }
    
    void emit(Args... args) const {
        for (const auto& [id, handler] : m_handlers) {
            handler(args...);
        }
    }

private:
    std::flat_map<HandlerId, Handler> m_handlers;
    HandlerId m_next_id = 0;
};

// include/core/Timer.h
class Timer {
public:
    Event<> onStarted;
    Event<> onPaused;
    Event<> onCompleted;
    Event<int> onTick;  // remaining seconds
    Event<TimerMode> onModeChanged;
    
    void start() {
        m_running = true;
        onStarted.emit();
    }
    
    void update() {
        if (m_running && hasTimeElapsed()) {
            --m_remaining;
            onTick.emit(m_remaining);
            
            if (m_remaining <= 0) {
                m_running = false;
                onCompleted.emit();
            }
        }
    }
};

// Usage in Application
m_timer.onCompleted.subscribe([this]() {
    m_audio->play(SoundEffect::TimerComplete);
    advanceToNextMode();
});

m_timer.onTick.subscribe([this](int remaining) {
    updateWindowTitle(remaining);
    updateSystemTray(remaining);
});
```

---

### 4.2 State Management with Observer Pattern

```cpp
// include/core/Observable.h
template<typename T>
class Observable {
public:
    using Observer = std::function<void(const T&, const T&)>;  // old, new
    
    Observable(T initial = {}) : m_value(std::move(initial)) {}
    
    const T& get() const { return m_value; }
    
    void set(T new_value) {
        if (m_value != new_value) {
            T old_value = std::exchange(m_value, std::move(new_value));
            for (const auto& observer : m_observers) {
                observer(old_value, m_value);
            }
        }
    }
    
    void observe(Observer observer) {
        m_observers.push_back(std::move(observer));
    }

private:
    T m_value;
    std::vector<Observer> m_observers;
};

// Usage
Observable<bool> timer_running{false};
Observable<int> pomodoro_count{0};

timer_running.observe([](bool old_val, bool new_val) {
    updateSystemTrayIcon(new_val);
});

pomodoro_count.observe([](int old_count, int new_count) {
    if (new_count > old_count) {
        playCompletionSound();
    }
});
```

---

## 5. Implementation Priority

### Phase 1: Foundation (Week 1-2)
- [ ] Update CMakeLists.txt to C++23
- [ ] Add `std::expected` to Persistence
- [ ] Create `ITimeSource` interface for testing
- [ ] Create `IPersistenceService` interface
- [ ] Centralize magic numbers in Configuration.h

### Phase 2: Split Large Classes (Week 3-4)
- [ ] Extract `TimerController` from Application
- [ ] Extract `WellnessController` from Application  
- [ ] Extract `TaskController` from Application
- [ ] Split `MainWindowCallbacks` into focused groups

### Phase 3: UI Refactoring (Week 5-6)
- [ ] Extract `SettingsPopup` component
- [ ] Extract `TimerPanel` component
- [ ] Extract `TaskListPanel` component
- [ ] Split `AppState` into focused structs

### Phase 4: Modernization (Week 7-8)
- [ ] Convert to `std::format` for string formatting
- [ ] Apply `std::ranges` where beneficial
- [ ] Use `std::flat_map` for small collections
- [ ] Replace C-arrays with std::string

### Phase 5: Architecture (Week 9-10)
- [ ] Implement Event system
- [ ] Add Observable state management
- [ ] Consider adding nlohmann/json dependency
- [ ] Write unit tests using mock interfaces

---

## Appendix: File Structure After Refactoring

```
include/
├── app/
│   ├── Application.h
│   └── ui/
│       ├── MainWindowView.h
│       ├── OverlayView.h
│       ├── WellnessViews.h
│       ├── callbacks/
│       │   ├── TimerCallbacks.h
│       │   ├── TaskCallbacks.h
│       │   ├── SettingsCallbacks.h
│       │   └── WindowCallbacks.h
│       └── components/
│           ├── SettingsPopup.h
│           ├── TimerPanel.h
│           ├── TaskListPanel.h
│           └── HelpPopup.h
├── controllers/
│   ├── TimerController.h
│   ├── WellnessController.h
│   └── TaskController.h
├── core/
│   ├── Configuration.h
│   ├── Event.h
│   ├── ITimeSource.h
│   ├── IPersistenceService.h
│   ├── JsonParser.h (or use nlohmann/json)
│   ├── Observable.h
│   ├── Persistence.h
│   ├── Task.h
│   ├── Timer.h
│   ├── WellnessTimer.h
│   └── WellnessTypes.h
├── system/
│   ├── AudioManager.h
│   ├── IAudioService.h
│   ├── MainWindow.h
│   ├── OverlayWindow.h
│   ├── SystemTray.h
│   └── WindowBase.h
└── ui/
    ├── AppState.h
    ├── NavigationTabs.h
    ├── PomodoroUI.h
    └── state/
        ├── UIState.h
        ├── DragState.h
        ├── SettingsEditState.h
        └── TaskEditState.h
```

---

## Notes

- Each refactoring step should be a separate commit/PR for easy review
- Run full build after each change to catch regressions
- Consider adding unit tests alongside interface extractions
- The nlohmann/json library decision depends on project constraints (binary size, build time)
