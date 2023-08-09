#pragma once

#include <string>
#include <unordered_map>

enum SoundType {
	LoseSound = 0,
	WinSound
};

bool LoadAssets(const std::string assetsDir);
void UnloadAssets();
void PlaySound(const SoundType& sound);
