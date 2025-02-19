#ifndef ENGINE_H
#define ENGINE_H

#include <memory>

#include <SDL3/SDL.h>

namespace lum
{
	class Engine
	{
	public:
		float deltaTime{};
		float scaledDeltaTime{};
		float engineTime{};
		float timeScalar{1.0f};

	public:
		Engine();
		~Engine();

		static Engine &Get();
		bool Init();
		void Shutdown();

		void Input(SDL_Event *p_event);
		void Update();
		void Render();

	private:
		static std::unique_ptr<Engine> m_instance;
	};

}

#endif // !ENGINE_H