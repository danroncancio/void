#include "audio_manager.hpp"

#include <memory>

#include <SDL3/SDL_audio.h>

#include "engine.hpp"

namespace lum
{
	// AUDIO CHANNEL
	//

	AudioChannel::AudioChannel() = default;

	AudioChannel::~AudioChannel() = default;

	bool AudioChannel::Init(SDL_AudioDeviceID p_device)
	{
		m_device = p_device;

		return true;
	}

	void AudioChannel::Shutdown()
	{
		StopAll();
	}

	void AudioChannel::PlaySound(SDL_AudioSpec &p_audioSpec, uint8_t *p_buffer, uint32_t p_length)
	{
		SDL_AudioStream *stream = SDL_CreateAudioStream(&p_audioSpec, &p_audioSpec);
		if (!stream)
		{
			SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to create audio stream: %s", SDL_GetError());
			return;
		}

		SDL_PutAudioStreamData(stream, p_buffer, p_length);
		SDL_SetAudioStreamGain(stream, m_volume);
		SDL_BindAudioStream(m_device, stream);

		m_activeStreams.push_back(stream);
	}

	void AudioChannel::StopAll()
	{
		for (auto *stream : m_activeStreams)
		{
			SDL_UnbindAudioStream(stream);
			SDL_DestroyAudioStream(stream);
		}

		m_activeStreams.clear();
	}

	void AudioChannel::SetVolume(float p_volume)
	{
		m_volume = p_volume;
		for (auto *stream : m_activeStreams)
		{
			SDL_SetAudioStreamGain(stream, m_volume);
		}
	}

	float AudioChannel::GetVolume() const
	{
		return m_volume;
	}

	void AudioChannel::Pause()
	{
		for (auto *stream : m_activeStreams)
		{
			SDL_PauseAudioStreamDevice(stream);
		}
	}

	void AudioChannel::Resume()
	{
		for (auto *stream : m_activeStreams)
		{
			SDL_ResumeAudioStreamDevice(stream);
		}
	}

	// AUDIO MANAGER
	//

	AudioManager::AudioManager() = default;

	AudioManager::~AudioManager() = default;

	bool AudioManager::Init()
	{
		m_device = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, nullptr);
		if (!m_device)
		{
			SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to create/open audio device: %s", SDL_GetError());
			return false;
		}

		// TEMPORAL
		// 
		// Create test audio channel

		m_channels["MUSIC"] = std::make_unique<AudioChannel>();
		m_channels["SFX"] = std::make_unique<AudioChannel>();


		// Init channels

		for (auto &[_, channel] : m_channels)
		{
			channel->Init(m_device);
		}

		return true;
	}

	void AudioManager::Shutdown()
	{
		for (auto &[_, channel] : m_channels)
		{
			channel->Shutdown();
		}

		SDL_CloseAudioDevice(m_device);
	}

	void AudioManager::PlaySound(const std::string &p_tag, const std::string &p_channelTag)
	{
		auto sound = Engine::Get().assetManager.GetSound(p_tag.c_str());
		if (!sound)
		{
			SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to find sound %s to play", p_tag.c_str());
			return;
		}

		auto channelIt = m_channels.find(p_channelTag);
		if (channelIt == m_channels.end())
		{
			SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to find channel %s in storage", p_tag.c_str());
			return;
		}

		channelIt->second->PlaySound(sound->audioSpec, sound->buffer.data(), sound->length);
	}

	void AudioManager::SetChannelVolume(const std::string &p_tag, float p_volume)
	{
		if (m_channels.find(p_tag) != m_channels.end())
		{
			m_channels[p_tag]->SetVolume(p_volume);
		}
	}
}