#include <system/AudioManager.h>

#include "assets/embedded_resources.h"

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
    if (m_initialized) {
        ma_engine_uninit(&m_engine);
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

bool AudioManager::isInitialized() const noexcept {
    return m_initialized;
}

void AudioManager::playEmbeddedSound(const unsigned char* data, size_t size) {
    std::filesystem::path temp_path;
    try {
        temp_path = createTemporaryWavPath();
    } catch (const std::exception& ex) {
        std::cerr << "Failed to create temporary audio path: " << ex.what() << '\n';
        return;
    }

    std::ofstream temp_file(temp_path, std::ios::binary);
    if (!temp_file) {
        std::cerr << "Failed to open temporary audio file: " << temp_path << '\n';
        return;
    }

    temp_file.write(reinterpret_cast<const char*>(data), static_cast<std::streamsize>(size));
    temp_file.close();

    const std::string path_string = temp_path.string();
    const ma_result result = ma_engine_play_sound(&m_engine, path_string.c_str(), nullptr);
    if (result != MA_SUCCESS) {
        std::cerr << "Failed to play sound. Error code: " << result << '\n';
    }
}

std::unique_ptr<IAudioService> createAudioService() {
    return std::make_unique<AudioManager>();
}

} // namespace WorkBalance::System
