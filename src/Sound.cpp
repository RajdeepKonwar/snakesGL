#include "Sound.h"
#include <iostream>

Sound::Sound()
{
	m_soundEngine = createIrrKlangDevice();
	std::cout << std::endl;
	if (!m_soundEngine)
	{
		std::cerr << "Error starting up the sound engine!\n";
		exit(EXIT_FAILURE);
	}
}

Sound::~Sound()
{
	m_soundEngine->drop();
}

bool Sound::Play(std::string audioFilePath, AudioDimension dimension, bool playLooped) const
{
	switch (dimension)
	{
	case TwoDimensional:
		m_soundEngine->play2D(audioFilePath.c_str(), playLooped);
		break;
	case ThreeDimensional:
		m_soundEngine->play3D(audioFilePath.c_str(), m_soundPosition, playLooped);
		break;
	default:
		break;
	}
	
	return false;
}

void Sound::SetSoundPosition(float x, float y, float z)
{
	m_soundPosition = irrklang::vec3df( static_cast<irrklang::ik_f32>(x),
										static_cast<irrklang::ik_f32>(y),
										static_cast<irrklang::ik_f32>(z));
}

void Sound::SetSoundVolume(float volume)
{
	m_soundEngine->setSoundVolume(static_cast<irrklang::ik_f32>(volume));
}
