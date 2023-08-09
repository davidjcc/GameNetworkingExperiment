#include "assets.h"

#include <raylib.h>
#include <spdlog/spdlog.h>

#define LOG(MSG, ...) \
  { \
    spdlog::info("[ASSETS] "); \
    spdlog::info(MSG, __VA_ARGS__); \
  }

struct Assets {
	std::unordered_map<SoundType, Sound> sounds;
};

static Assets s_assets;
static std::string s_assetsDir;

static std::string SoundIDToString(const SoundType& type) {
	switch (type) {
	case LoseSound: return "LoseSound";
	case WinSound: return "WinSound";
	default: return "<unknown>";
	}

}

#define LOAD_SOUND(SOUND_PATH, SOUND_ID) \
  { \
    std::string path = s_assetsDir + "/sounds/" + SOUND_PATH; \
    LOG("Loading sound: {} - {}", SoundIDToString(SOUND_ID), path); \
    s_assets.sounds[SOUND_ID] = LoadSound(path.c_str()); \
  }


bool LoadAssets(const std::string assetsDir) {
	s_assetsDir = assetsDir;

	LOAD_SOUND("lose.mp3", LoseSound);
	LOAD_SOUND("win.mp3", WinSound);

	return true;
}

void UnloadAssets() {
	for (auto& [_, sound] : s_assets.sounds) {
		UnloadSound(sound);
	}
}

void PlaySound(const SoundType& sound) {
	if (auto it = s_assets.sounds.find(sound); it != s_assets.sounds.end()) {
		PlaySound(it->second);
	}
}
