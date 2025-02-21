#ifndef AUDIO_MANAGER_H
#define AUDIO_MANAGER_H

namespace lum
{
	class AudioChannel
	{
	public:
		AudioChannel();
		~AudioChannel();

		bool Init(SDL_AudioDeviceID p_device);
		void Shutdown();

		void PlaySound(SDL_AudioSpec &p_audioSpec, uint8_t *p_buffer, uint32_t p_length);
		void StopAll();
		void SetVolume(float p_newVolume);
		float GetVolume() const;
		void Pause();
		void Resume();

	private:
		SDL_AudioDeviceID m_device{};
		float m_volume{ 1.0f };
		std::vector<SDL_AudioStream *> m_activeStreams{};
	};

	class AudioManager
	{
	public:
		AudioManager();
		~AudioManager();

		bool Init();
		void Shutdown();

		void PlaySound(const std::string &p_tag, const std::string &p_channelTag);
		void SetChannelVolume(const std::string &p_tag, float p_volume);

	private:
		SDL_AudioDeviceID m_device{};
		std::unordered_map<std::string, std::unique_ptr<AudioChannel>> m_channels;

	private:
		AudioManager(const AudioManager &) = delete;
		AudioManager &operator=(const AudioManager &) = delete;
	};
}

#endif // !AUDIO_MANAGER_H