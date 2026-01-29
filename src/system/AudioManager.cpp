#include <system/AudioManager.h>

#include "assets/embedded_resources.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <random>
#include <string>

namespace WorkBalance::System {

namespace {
[[nodiscard]] std::filesystem::path createTemporaryWavPath() {
    static constexpr char hex_digits[] = "0123456789abcdef";

    std::random_device rd;
    std::mt19937 generator(rd());
    std::uniform_int_distribution<int> distribution(0, 15);

    std::string filename = "workbalance-";
    for (int i = 0; i < 12; ++i) {
        filename += hex_digits[distribution(generator)];
    }
    filename += ".wav";

    return std::filesystem::temp_directory_path() / filename;
}
} // namespace

AudioManager::AudioManager() {
    const ma_result result = ma_engine_init(nullptr, &m_engine);
    if (result != MA_SUCCESS) {
        std::cerr << "Failed to initialize audio engine. Error code: " << result << '\n';
        m_initialized = false;
    } else {
        m_initialized = true;
    }
}

AudioManager::~AudioManager() {
    stopNotificationSounds();
    if (m_initialized) {
        ma_engine_uninit(&m_engine);
    }
}

void AudioManager::stopNotificationSounds() {
    if (m_notification_sound_initialized) {
        ma_sound_stop(&m_notification_sound);
        ma_sound_uninit(&m_notification_sound);
        m_notification_sound_initialized = false;
    }
    cleanupTempFile();
}

void AudioManager::cleanupTempFile() {
    if (!m_current_temp_path.empty()) {
        std::error_code ec;
        std::filesystem::remove(m_current_temp_path, ec);
        m_current_temp_path.clear();
    }
}

void AudioManager::playClickSound() {
    if (!m_initialized) {
        return;
    }

    playEmbeddedSound(click_wav_data, click_wav_data_size);
}

void AudioManager::playBellSound() {
    if (!m_initialized) {
        return;
    }

    playEmbeddedSound(bell_wav_data, bell_wav_data_size);
}

void AudioManager::playHydrationSound() {
    if (!m_initialized) {
        return;
    }

    playEmbeddedSound(hydration_wav_data, hydration_wav_data_size);
}

void AudioManager::playWalkSound() {
    if (!m_initialized) {
        return;
    }

    playEmbeddedSound(walk_wav_data, walk_wav_data_size);
}

bool AudioManager::isInitialized() const noexcept {
    return m_initialized;
}

void AudioManager::setVolume(int volume) {
    m_volume = std::clamp(volume, 0, 100);
    if (m_initialized) {
        // Convert 0-100 to 0.0-1.0 for miniaudio
        ma_engine_set_volume(&m_engine, static_cast<float>(m_volume) / 100.0f);
    }
}

int AudioManager::getVolume() const noexcept {
    return m_volume;
}

void AudioManager::playEmbeddedSound(const unsigned char* data, size_t size) {
    // Stop any currently playing notification sound and clean up
    stopNotificationSounds();

    try {
        m_current_temp_path = createTemporaryWavPath();
    } catch (const std::exception& ex) {
        std::cerr << "Failed to create temporary audio path: " << ex.what() << '\n';
        return;
    }

    std::ofstream temp_file(m_current_temp_path, std::ios::binary);
    if (!temp_file) {
        std::cerr << "Failed to open temporary audio file: " << m_current_temp_path << '\n';
        m_current_temp_path.clear();
        return;
    }

    temp_file.write(reinterpret_cast<const char*>(data), static_cast<std::streamsize>(size));
    temp_file.close();

    const std::string path_string = m_current_temp_path.string();

    // Initialize sound with explicit no-looping flag
    constexpr ma_uint32 flags = MA_SOUND_FLAG_ASYNC | MA_SOUND_FLAG_NO_SPATIALIZATION;
    ma_result result =
        ma_sound_init_from_file(&m_engine, path_string.c_str(), flags, nullptr, nullptr, &m_notification_sound);
    if (result != MA_SUCCESS) {
        std::cerr << "Failed to initialize sound. Error code: " << result << '\n';
        cleanupTempFile();
        return;
    }

    m_notification_sound_initialized = true;

    // Explicitly disable looping
    ma_sound_set_looping(&m_notification_sound, MA_FALSE);

    result = ma_sound_start(&m_notification_sound);
    if (result != MA_SUCCESS) {
        std::cerr << "Failed to play sound. Error code: " << result << '\n';
        stopNotificationSounds();
    }
}

std::unique_ptr<IAudioService> createAudioService() {
    return std::make_unique<AudioManager>();
}

} // namespace WorkBalance::System
