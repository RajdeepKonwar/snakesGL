#pragma once

#include <string>
#include "irrKlang.h"

using namespace irrklang;
#pragma comment(lib, "irrKlang.lib")

enum AudioDimension
{
	TwoDimensional,
	ThreeDimensional
};

class Sound {
public:
	Sound();
	~Sound();

	bool Play(std::string audioFilePath, AudioDimension dimension = TwoDimensional, bool playLooped = false) const;
	void SetSoundPosition(float x, float y, float z);
	void SetSoundVolume(float volume);

private:
	ISoundEngine* m_soundEngine;
	std::string m_audioFilePath;

	irrklang::vec3df m_soundPosition = irrklang::vec3df(static_cast<irrklang::ik_f32>(0.0),
														static_cast<irrklang::ik_f32>(0.0),
														static_cast<irrklang::ik_f32>(0.0));
};