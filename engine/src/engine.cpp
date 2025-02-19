#include "engine.hpp"

namespace lum
{
	std::unique_ptr<Engine> Engine::m_instance = nullptr;

	Engine::Engine() = default;

	Engine::~Engine() = default;

	Engine &Engine::Get()
	{
		if (!m_instance)
		{
			m_instance = std::make_unique<Engine>();
		}

		return *m_instance;
	}

	bool Engine::Init()
	{
		return true;
	}

	void Engine::Shutdown()
	{

	}

	void Engine::Input(SDL_Event *p_event)
	{
		switch (p_event->type)
		{
		default:
			break;
		}
	}

	void Engine::Update()
	{

	}

	void Engine::Render()
	{

	}
}