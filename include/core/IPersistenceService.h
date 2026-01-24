#pragma once

#include "Persistence.h"

#include <expected>
#include <filesystem>
#include <memory>

namespace WorkBalance::Core {

/// @brief Interface for persistence operations, enabling testability and different storage backends
class IPersistenceService {
  public:
    virtual ~IPersistenceService() = default;

    /// @brief Saves the current application state
    /// @param data The data to persist
    /// @return std::expected with void on success, or PersistenceError on failure
    [[nodiscard]] virtual std::expected<void, PersistenceError> save(const PersistentData& data) const = 0;

    /// @brief Loads previously saved application state
    /// @return std::expected with PersistentData on success, or PersistenceError on failure
    [[nodiscard]] virtual std::expected<PersistentData, PersistenceError> load() const = 0;

    /// @brief Checks if a saved state exists
    [[nodiscard]] virtual bool hasSavedData() const = 0;
};

/// @brief JSON file-based implementation of IPersistenceService
/// Uses PersistenceManager internally for backward compatibility
class JsonPersistenceService final : public IPersistenceService {
  public:
    JsonPersistenceService() : m_manager() {
    }

    explicit JsonPersistenceService(std::filesystem::path config_directory) : m_manager(std::move(config_directory)) {
    }

    [[nodiscard]] std::expected<void, PersistenceError> save(const PersistentData& data) const override {
        return m_manager.save(data);
    }

    [[nodiscard]] std::expected<PersistentData, PersistenceError> load() const override {
        return m_manager.load();
    }

    [[nodiscard]] bool hasSavedData() const override {
        return m_manager.hasSavedData();
    }

    /// @brief Gets the path to the configuration file
    [[nodiscard]] const std::filesystem::path& getConfigPath() const noexcept {
        return m_manager.getConfigPath();
    }

  private:
    PersistenceManager m_manager;
};

/// @brief Mock implementation for testing
class MockPersistenceService final : public IPersistenceService {
  public:
    [[nodiscard]] std::expected<void, PersistenceError> save(const PersistentData& data) const override {
        m_saved_data = data;
        m_has_data = true;
        if (m_force_save_error) {
            return std::unexpected(*m_force_save_error);
        }
        return {};
    }

    [[nodiscard]] std::expected<PersistentData, PersistenceError> load() const override {
        if (m_force_load_error) {
            return std::unexpected(*m_force_load_error);
        }
        if (!m_has_data) {
            return std::unexpected(PersistenceError::FileNotFound);
        }
        return m_saved_data;
    }

    [[nodiscard]] bool hasSavedData() const override {
        return m_has_data;
    }

    // Test helpers
    void setData(const PersistentData& data) {
        m_saved_data = data;
        m_has_data = true;
    }

    void clearData() {
        m_saved_data = PersistentData{};
        m_has_data = false;
    }

    void forceSaveError(PersistenceError error) {
        m_force_save_error = error;
    }

    void forceLoadError(PersistenceError error) {
        m_force_load_error = error;
    }

    void clearForcedErrors() {
        m_force_save_error = std::nullopt;
        m_force_load_error = std::nullopt;
    }

    [[nodiscard]] const PersistentData& getSavedData() const {
        return m_saved_data;
    }

  private:
    mutable PersistentData m_saved_data{};
    mutable bool m_has_data = false;
    std::optional<PersistenceError> m_force_save_error;
    std::optional<PersistenceError> m_force_load_error;
};

/// @brief Factory function to create the default persistence service
[[nodiscard]] inline std::unique_ptr<IPersistenceService> createDefaultPersistenceService() {
    return std::make_unique<JsonPersistenceService>();
}

} // namespace WorkBalance::Core
