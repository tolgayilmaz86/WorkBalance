#include <core/Persistence.h>

#include <algorithm>
#include <cstdlib>
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

bool PersistenceManager::save(const PersistentData& data) const {
    try {
        // Ensure directory exists
        const auto directory = m_config_path.parent_path();
        if (!std::filesystem::exists(directory)) {
            std::filesystem::create_directories(directory);
        }

        const std::string json = serializeToJson(data);

        std::ofstream file(m_config_path);
        if (!file.is_open()) {
            std::cerr << "Failed to open config file for writing: " << m_config_path << '\n';
            return false;
        }

        file << json;
        return file.good();
    } catch (const std::exception& e) {
        std::cerr << "Error saving configuration: " << e.what() << '\n';
        return false;
    }
}

std::optional<PersistentData> PersistenceManager::load() const {
    try {
        if (!hasSavedData()) {
            return std::nullopt;
        }

        std::ifstream file(m_config_path);
        if (!file.is_open()) {
            return std::nullopt;
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        const std::string json = buffer.str();

        return deserializeFromJson(json);
    } catch (const std::exception& e) {
        std::cerr << "Error loading configuration: " << e.what() << '\n';
        return std::nullopt;
    }
}

bool PersistenceManager::hasSavedData() const {
    return std::filesystem::exists(m_config_path);
}

std::string PersistenceManager::serializeToJson(const PersistentData& data) {
    std::ostringstream json;
    json << "{\n";

    // Settings section
    json << "  \"settings\": {\n";
    json << "    \"pomodoro_duration_minutes\": " << data.settings.pomodoro_duration_minutes << ",\n";
    json << "    \"short_break_duration_minutes\": " << data.settings.short_break_duration_minutes << ",\n";
    json << "    \"long_break_duration_minutes\": " << data.settings.long_break_duration_minutes << ",\n";
    json << "    \"auto_start_breaks\": " << (data.settings.auto_start_breaks ? "true" : "false") << ",\n";
    json << "    \"auto_start_pomodoros\": " << (data.settings.auto_start_pomodoros ? "true" : "false") << ",\n";
    json << "    \"overlay_position_x\": " << data.settings.overlay_position_x << ",\n";
    json << "    \"overlay_position_y\": " << data.settings.overlay_position_y << ",\n";
    json << "    \"main_window_x\": " << data.settings.main_window_x << ",\n";
    json << "    \"main_window_y\": " << data.settings.main_window_y << ",\n";
    json << "    \"show_pomodoro_in_overlay\": " << (data.settings.show_pomodoro_in_overlay ? "true" : "false")
         << ",\n";
    json << "    \"show_water_in_overlay\": " << (data.settings.show_water_in_overlay ? "true" : "false") << ",\n";
    json << "    \"show_standup_in_overlay\": " << (data.settings.show_standup_in_overlay ? "true" : "false") << ",\n";
    json << "    \"show_eye_care_in_overlay\": " << (data.settings.show_eye_care_in_overlay ? "true" : "false") << "\n";
    json << "  },\n";

    // Current task index
    json << "  \"current_task_index\": " << data.current_task_index << ",\n";

    // Tasks array
    json << "  \"tasks\": [\n";
    for (size_t i = 0; i < data.tasks.size(); ++i) {
        const auto& task = data.tasks[i];
        json << "    {\n";
        json << "      \"name\": \"" << escapeJsonString(task.name) << "\",\n";
        json << "      \"completed\": " << (task.completed ? "true" : "false") << ",\n";
        json << "      \"estimated_pomodoros\": " << task.estimated_pomodoros << ",\n";
        json << "      \"completed_pomodoros\": " << task.completed_pomodoros << "\n";
        json << "    }";
        if (i < data.tasks.size() - 1) {
            json << ",";
        }
        json << "\n";
    }
    json << "  ]\n";

    json << "}\n";
    return json.str();
}

std::optional<PersistentData> PersistenceManager::deserializeFromJson(const std::string& json) {
    PersistentData data;

    // Extract settings section
    const std::string settings_json = extractJsonValue(json, "settings");
    if (!settings_json.empty()) {
        data.settings.pomodoro_duration_minutes = extractJsonInt(settings_json, "pomodoro_duration_minutes", 25);
        data.settings.short_break_duration_minutes = extractJsonInt(settings_json, "short_break_duration_minutes", 5);
        data.settings.long_break_duration_minutes = extractJsonInt(settings_json, "long_break_duration_minutes", 15);
        data.settings.auto_start_breaks = extractJsonBool(settings_json, "auto_start_breaks", false);
        data.settings.auto_start_pomodoros = extractJsonBool(settings_json, "auto_start_pomodoros", false);
        data.settings.overlay_position_x = extractJsonFloat(settings_json, "overlay_position_x", 100.0f);
        data.settings.overlay_position_y = extractJsonFloat(settings_json, "overlay_position_y", 100.0f);
        data.settings.main_window_x = extractJsonInt(settings_json, "main_window_x", -1);
        data.settings.main_window_y = extractJsonInt(settings_json, "main_window_y", -1);
        data.settings.show_pomodoro_in_overlay = extractJsonBool(settings_json, "show_pomodoro_in_overlay", true);
        data.settings.show_water_in_overlay = extractJsonBool(settings_json, "show_water_in_overlay", true);
        data.settings.show_standup_in_overlay = extractJsonBool(settings_json, "show_standup_in_overlay", true);
        data.settings.show_eye_care_in_overlay = extractJsonBool(settings_json, "show_eye_care_in_overlay", true);
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
            task.estimated_pomodoros = extractJsonInt(task_json, "estimated_pomodoros", 1);
            task.completed_pomodoros = extractJsonInt(task_json, "completed_pomodoros", 0);
            data.tasks.push_back(std::move(task));
        }
    }

    return data;
}

} // namespace WorkBalance::Core
