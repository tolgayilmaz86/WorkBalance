#include <core/Persistence.h>

#include <core/Configuration.h>

#include <algorithm>
#include <cstdlib>
#include <format>
#include <fstream>
#include <iostream>
#include <sstream>

#ifdef _WIN32
#include <ShlObj.h>
#include <Windows.h>
#endif

namespace WorkBalance::Core {

namespace {
constexpr std::string_view CONFIG_FILENAME = "workbalance_config.json";
constexpr std::string_view APP_FOLDER_NAME = "WorkBalance";

// Simple JSON helper functions (avoiding external dependency)
std::string escapeJsonString(const std::string& input) {
    std::string output;
    output.reserve(input.size() + 10);
    for (char ch : input) {
        switch (ch) {
            case '"':
                output += "\\\"";
                break;
            case '\\':
                output += "\\\\";
                break;
            case '\n':
                output += "\\n";
                break;
            case '\r':
                output += "\\r";
                break;
            case '\t':
                output += "\\t";
                break;
            default:
                output += ch;
                break;
        }
    }
    return output;
}

std::string unescapeJsonString(const std::string& input) {
    std::string output;
    output.reserve(input.size());
    for (size_t i = 0; i < input.size(); ++i) {
        if (input[i] == '\\' && i + 1 < input.size()) {
            switch (input[i + 1]) {
                case '"':
                    output += '"';
                    ++i;
                    break;
                case '\\':
                    output += '\\';
                    ++i;
                    break;
                case 'n':
                    output += '\n';
                    ++i;
                    break;
                case 'r':
                    output += '\r';
                    ++i;
                    break;
                case 't':
                    output += '\t';
                    ++i;
                    break;
                default:
                    output += input[i];
                    break;
            }
        } else {
            output += input[i];
        }
    }
    return output;
}

std::string trim(const std::string& str) {
    const auto start = str.find_first_not_of(" \t\n\r");
    if (start == std::string::npos) {
        return "";
    }
    const auto end = str.find_last_not_of(" \t\n\r");
    return str.substr(start, end - start + 1);
}

std::string extractJsonValue(const std::string& json, const std::string& key) {
    const std::string search_key = "\"" + key + "\"";
    auto pos = json.find(search_key);
    if (pos == std::string::npos) {
        return "";
    }

    pos = json.find(':', pos + search_key.size());
    if (pos == std::string::npos) {
        return "";
    }

    pos = json.find_first_not_of(" \t\n\r", pos + 1);
    if (pos == std::string::npos) {
        return "";
    }

    if (json[pos] == '"') {
        // String value
        auto end_pos = pos + 1;
        while (end_pos < json.size()) {
            if (json[end_pos] == '"' && json[end_pos - 1] != '\\') {
                break;
            }
            ++end_pos;
        }
        return unescapeJsonString(json.substr(pos + 1, end_pos - pos - 1));
    }
    if (json[pos] == '{') {
        // Object value - find matching brace
        int brace_count = 1;
        auto end_pos = pos + 1;
        while (end_pos < json.size() && brace_count > 0) {
            if (json[end_pos] == '{') {
                ++brace_count;
            } else if (json[end_pos] == '}') {
                --brace_count;
            }
            ++end_pos;
        }
        return json.substr(pos, end_pos - pos);
    }
    if (json[pos] == '[') {
        // Array value - find matching bracket
        int bracket_count = 1;
        auto end_pos = pos + 1;
        while (end_pos < json.size() && bracket_count > 0) {
            if (json[end_pos] == '[') {
                ++bracket_count;
            } else if (json[end_pos] == ']') {
                --bracket_count;
            }
            ++end_pos;
        }
        return json.substr(pos, end_pos - pos);
    }
    // Numeric or boolean value
    auto end_pos = json.find_first_of(",}\n\r", pos);
    if (end_pos == std::string::npos) {
        end_pos = json.size();
    }
    return trim(json.substr(pos, end_pos - pos));
}

int extractJsonInt(const std::string& json, const std::string& key, int default_value) {
    const std::string value = extractJsonValue(json, key);
    if (value.empty()) {
        return default_value;
    }
    try {
        return std::stoi(value);
    } catch (...) {
        return default_value;
    }
}

float extractJsonFloat(const std::string& json, const std::string& key, float default_value) {
    const std::string value = extractJsonValue(json, key);
    if (value.empty()) {
        return default_value;
    }
    try {
        return std::stof(value);
    } catch (...) {
        return default_value;
    }
}

bool extractJsonBool(const std::string& json, const std::string& key, bool default_value) {
    const std::string value = extractJsonValue(json, key);
    if (value.empty()) {
        return default_value;
    }
    return value == "true";
}

std::vector<std::string> splitJsonArray(const std::string& array_json) {
    std::vector<std::string> items;
    if (array_json.size() < 2 || array_json.front() != '[' || array_json.back() != ']') {
        return items;
    }

    std::string content = array_json.substr(1, array_json.size() - 2);
    int brace_count = 0;
    size_t item_start = 0;

    for (size_t i = 0; i < content.size(); ++i) {
        if (content[i] == '{') {
            if (brace_count == 0) {
                item_start = i;
            }
            ++brace_count;
        } else if (content[i] == '}') {
            --brace_count;
            if (brace_count == 0) {
                items.push_back(content.substr(item_start, i - item_start + 1));
            }
        }
    }

    return items;
}
} // namespace

PersistenceManager::PersistenceManager() : PersistenceManager(getDefaultConfigDirectory()) {
}

PersistenceManager::PersistenceManager(std::filesystem::path config_directory)
    : m_config_path(std::move(config_directory) / CONFIG_FILENAME) {
}

std::filesystem::path PersistenceManager::getDefaultConfigDirectory() {
#ifdef _WIN32
    wchar_t* path_ptr = nullptr;
    if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, nullptr, &path_ptr))) {
        std::filesystem::path base_path(path_ptr);
        CoTaskMemFree(path_ptr);
        return base_path / APP_FOLDER_NAME;
    }
    // Fallback to LOCALAPPDATA environment variable
    char* appdata = nullptr;
    size_t len = 0;
    if (_dupenv_s(&appdata, &len, "LOCALAPPDATA") == 0 && appdata != nullptr) {
        std::filesystem::path result(appdata);
        free(appdata);
        return result / APP_FOLDER_NAME;
    }
#else
    // Linux/macOS: use XDG_CONFIG_HOME or ~/.config
    if (const char* xdg_config = std::getenv("XDG_CONFIG_HOME")) {
        return std::filesystem::path(xdg_config) / APP_FOLDER_NAME;
    }
    if (const char* home = std::getenv("HOME")) {
        return std::filesystem::path(home) / ".config" / APP_FOLDER_NAME;
    }
#endif
    // Last resort: current directory
    return std::filesystem::current_path() / APP_FOLDER_NAME;
}

std::expected<void, PersistenceError> PersistenceManager::save(const PersistentData& data) const {
    try {
        // Ensure directory exists
        const auto directory = m_config_path.parent_path();
        if (!std::filesystem::exists(directory)) {
            if (!std::filesystem::create_directories(directory)) {
                std::cerr << "Failed to create config directory: " << directory << '\n';
                return std::unexpected(PersistenceError::DirectoryCreateError);
            }
        }

        const std::string json = serializeToJson(data);

        std::ofstream file(m_config_path);
        if (!file.is_open()) {
            std::cerr << "Failed to open config file for writing: " << m_config_path << '\n';
            return std::unexpected(PersistenceError::FileOpenError);
        }

        file << json;
        if (!file.good()) {
            return std::unexpected(PersistenceError::WriteError);
        }
        return {};
    } catch (const std::exception& e) {
        std::cerr << "Error saving configuration: " << e.what() << '\n';
        return std::unexpected(PersistenceError::WriteError);
    }
}

std::expected<PersistentData, PersistenceError> PersistenceManager::load() const {
    try {
        if (!hasSavedData()) {
            return std::unexpected(PersistenceError::FileNotFound);
        }

        std::ifstream file(m_config_path);
        if (!file.is_open()) {
            return std::unexpected(PersistenceError::FileOpenError);
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        const std::string json = buffer.str();

        auto result = deserializeFromJson(json);
        if (!result) {
            return std::unexpected(PersistenceError::ParseError);
        }
        return *result;
    } catch (const std::exception& e) {
        std::cerr << "Error loading configuration: " << e.what() << '\n';
        return std::unexpected(PersistenceError::ParseError);
    }
}

bool PersistenceManager::hasSavedData() const {
    return std::filesystem::exists(m_config_path);
}

std::string PersistenceManager::serializeToJson(const PersistentData& data) {
    // Format tasks array
    std::string tasks_json;
    for (size_t i = 0; i < data.tasks.size(); ++i) {
        const auto& task = data.tasks[i];
        tasks_json += std::format(
            R"(    {{
      "name": "{}",
      "completed": {},
      "estimated_pomodoros": {},
      "completed_pomodoros": {}
    }}{}
)",
            escapeJsonString(task.name), task.completed ? "true" : "false", task.estimated_pomodoros,
            task.completed_pomodoros, (i < data.tasks.size() - 1) ? "," : "");
    }

    return std::format(
        R"({{
  "settings": {{
    "pomodoro_duration_minutes": {},
    "short_break_duration_minutes": {},
    "long_break_duration_minutes": {},
    "auto_start_breaks": {},
    "auto_start_pomodoros": {},
    "pomodoros_before_long_break": {},
    "long_breaks_in_cycle": {},
    "overlay_position_x": {},
    "overlay_position_y": {},
    "main_window_x": {},
    "main_window_y": {},
    "show_pomodoro_in_overlay": {},
    "show_water_in_overlay": {},
    "show_standup_in_overlay": {},
    "show_eye_care_in_overlay": {},
    "water_interval_minutes": {},
    "water_daily_goal": {},
    "standup_interval_minutes": {},
    "standup_duration_minutes": {},
    "eye_care_interval_minutes": {},
    "eye_care_break_seconds": {},
    "water_auto_loop": {},
    "standup_auto_loop": {},
    "eye_care_auto_loop": {},
    "start_with_windows": {},
    "start_minimized": {},
    "pomodoro_sound_enabled": {},
    "pomodoro_sound_volume": {},
    "water_sound_enabled": {},
    "water_sound_volume": {},
    "standup_sound_enabled": {},
    "standup_sound_volume": {},
    "eye_care_sound_enabled": {},
    "eye_care_sound_volume": {},
    "pomodoro_notification_enabled": {},
    "water_notification_enabled": {},
    "standup_notification_enabled": {},
    "eye_care_notification_enabled": {}
  }},
  "current_task_index": {},
  "tasks": [
{}  ]
}}
)",
        data.settings.pomodoro_duration_minutes, data.settings.short_break_duration_minutes,
        data.settings.long_break_duration_minutes, data.settings.auto_start_breaks ? "true" : "false",
        data.settings.auto_start_pomodoros ? "true" : "false", data.settings.pomodoros_before_long_break,
        data.settings.long_breaks_in_cycle, data.settings.overlay_position_x, data.settings.overlay_position_y,
        data.settings.main_window_x, data.settings.main_window_y,
        data.settings.show_pomodoro_in_overlay ? "true" : "false",
        data.settings.show_water_in_overlay ? "true" : "false",
        data.settings.show_standup_in_overlay ? "true" : "false",
        data.settings.show_eye_care_in_overlay ? "true" : "false", data.settings.water_interval_minutes,
        data.settings.water_daily_goal, data.settings.standup_interval_minutes, data.settings.standup_duration_minutes,
        data.settings.eye_care_interval_minutes, data.settings.eye_care_break_seconds,
        data.settings.water_auto_loop ? "true" : "false", data.settings.standup_auto_loop ? "true" : "false",
        data.settings.eye_care_auto_loop ? "true" : "false", data.settings.start_with_windows ? "true" : "false",
        data.settings.start_minimized ? "true" : "false", data.settings.pomodoro_sound_enabled ? "true" : "false",
        data.settings.pomodoro_sound_volume, data.settings.water_sound_enabled ? "true" : "false",
        data.settings.water_sound_volume, data.settings.standup_sound_enabled ? "true" : "false",
        data.settings.standup_sound_volume, data.settings.eye_care_sound_enabled ? "true" : "false",
        data.settings.eye_care_sound_volume, data.settings.pomodoro_notification_enabled ? "true" : "false",
        data.settings.water_notification_enabled ? "true" : "false",
        data.settings.standup_notification_enabled ? "true" : "false",
        data.settings.eye_care_notification_enabled ? "true" : "false", data.current_task_index, tasks_json);
}

std::optional<PersistentData> PersistenceManager::deserializeFromJson(const std::string& json) {
    PersistentData data;

    // Extract settings section
    const std::string settings_json = extractJsonValue(json, "settings");
    if (!settings_json.empty()) {
        data.settings.pomodoro_duration_minutes =
            extractJsonInt(settings_json, "pomodoro_duration_minutes", Configuration::DEFAULT_POMODORO_MINUTES);
        data.settings.short_break_duration_minutes =
            extractJsonInt(settings_json, "short_break_duration_minutes", Configuration::DEFAULT_SHORT_BREAK_MINUTES);
        data.settings.long_break_duration_minutes =
            extractJsonInt(settings_json, "long_break_duration_minutes", Configuration::DEFAULT_LONG_BREAK_MINUTES);
        data.settings.auto_start_breaks = extractJsonBool(settings_json, "auto_start_breaks", false);
        data.settings.auto_start_pomodoros = extractJsonBool(settings_json, "auto_start_pomodoros", false);
        data.settings.pomodoros_before_long_break = extractJsonInt(settings_json, "pomodoros_before_long_break",
                                                                   Configuration::DEFAULT_POMODOROS_BEFORE_LONG_BREAK);
        data.settings.long_breaks_in_cycle =
            extractJsonInt(settings_json, "long_breaks_in_cycle", Configuration::DEFAULT_LONG_BREAKS_IN_CYCLE);
        data.settings.overlay_position_x =
            extractJsonFloat(settings_json, "overlay_position_x", Configuration::DEFAULT_OVERLAY_POSITION_X);
        data.settings.overlay_position_y =
            extractJsonFloat(settings_json, "overlay_position_y", Configuration::DEFAULT_OVERLAY_POSITION_Y);
        data.settings.main_window_x =
            extractJsonInt(settings_json, "main_window_x", Configuration::DEFAULT_WINDOW_POSITION);
        data.settings.main_window_y =
            extractJsonInt(settings_json, "main_window_y", Configuration::DEFAULT_WINDOW_POSITION);
        data.settings.show_pomodoro_in_overlay = extractJsonBool(settings_json, "show_pomodoro_in_overlay", true);
        data.settings.show_water_in_overlay = extractJsonBool(settings_json, "show_water_in_overlay", true);
        data.settings.show_standup_in_overlay = extractJsonBool(settings_json, "show_standup_in_overlay", true);
        data.settings.show_eye_care_in_overlay = extractJsonBool(settings_json, "show_eye_care_in_overlay", true);
        // Wellness timer settings
        data.settings.water_interval_minutes = extractJsonInt(settings_json, "water_interval_minutes", 30);
        data.settings.water_daily_goal = extractJsonInt(settings_json, "water_daily_goal", 8);
        data.settings.standup_interval_minutes = extractJsonInt(settings_json, "standup_interval_minutes", 45);
        data.settings.standup_duration_minutes = extractJsonInt(settings_json, "standup_duration_minutes", 5);
        data.settings.eye_care_interval_minutes = extractJsonInt(settings_json, "eye_care_interval_minutes", 20);
        data.settings.eye_care_break_seconds = extractJsonInt(settings_json, "eye_care_break_seconds", 20);
        // Wellness auto-loop settings
        data.settings.water_auto_loop = extractJsonBool(settings_json, "water_auto_loop", false);
        data.settings.standup_auto_loop = extractJsonBool(settings_json, "standup_auto_loop", false);
        data.settings.eye_care_auto_loop = extractJsonBool(settings_json, "eye_care_auto_loop", false);
        // Startup settings
        data.settings.start_with_windows = extractJsonBool(settings_json, "start_with_windows", false);
        data.settings.start_minimized = extractJsonBool(settings_json, "start_minimized", true);
        // Sound settings
        data.settings.pomodoro_sound_enabled =
            extractJsonBool(settings_json, "pomodoro_sound_enabled", Configuration::DEFAULT_SOUND_ENABLED);
        data.settings.pomodoro_sound_volume =
            extractJsonInt(settings_json, "pomodoro_sound_volume", Configuration::DEFAULT_SOUND_VOLUME);
        data.settings.water_sound_enabled =
            extractJsonBool(settings_json, "water_sound_enabled", Configuration::DEFAULT_SOUND_ENABLED);
        data.settings.water_sound_volume =
            extractJsonInt(settings_json, "water_sound_volume", Configuration::DEFAULT_SOUND_VOLUME);
        data.settings.standup_sound_enabled =
            extractJsonBool(settings_json, "standup_sound_enabled", Configuration::DEFAULT_SOUND_ENABLED);
        data.settings.standup_sound_volume =
            extractJsonInt(settings_json, "standup_sound_volume", Configuration::DEFAULT_SOUND_VOLUME);
        data.settings.eye_care_sound_enabled =
            extractJsonBool(settings_json, "eye_care_sound_enabled", Configuration::DEFAULT_SOUND_ENABLED);
        data.settings.eye_care_sound_volume =
            extractJsonInt(settings_json, "eye_care_sound_volume", Configuration::DEFAULT_SOUND_VOLUME);
        // Notification settings
        data.settings.pomodoro_notification_enabled = extractJsonBool(settings_json, "pomodoro_notification_enabled",
                                                                      Configuration::DEFAULT_NOTIFICATIONS_ENABLED);
        data.settings.water_notification_enabled =
            extractJsonBool(settings_json, "water_notification_enabled", Configuration::DEFAULT_NOTIFICATIONS_ENABLED);
        data.settings.standup_notification_enabled = extractJsonBool(settings_json, "standup_notification_enabled",
                                                                     Configuration::DEFAULT_NOTIFICATIONS_ENABLED);
        data.settings.eye_care_notification_enabled = extractJsonBool(settings_json, "eye_care_notification_enabled",
                                                                      Configuration::DEFAULT_NOTIFICATIONS_ENABLED);
    }

    // Extract current task index
    data.current_task_index = extractJsonInt(json, "current_task_index", 0);

    // Extract tasks array
    const std::string tasks_json = extractJsonValue(json, "tasks");
    if (!tasks_json.empty()) {
        const auto task_items = splitJsonArray(tasks_json);
        for (const auto& task_json : task_items) {
            Task task;
            task.name = extractJsonValue(task_json, "name");
            task.completed = extractJsonBool(task_json, "completed", false);
            task.estimated_pomodoros =
                extractJsonInt(task_json, "estimated_pomodoros", Configuration::DEFAULT_ESTIMATED_POMODOROS);
            task.completed_pomodoros =
                extractJsonInt(task_json, "completed_pomodoros", Configuration::DEFAULT_COMPLETED_POMODOROS);
            data.tasks.push_back(std::move(task));
        }
    }

    return data;
}

} // namespace WorkBalance::Core
