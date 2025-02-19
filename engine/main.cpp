#include "src/engine.cpp"
#include "src/scene.cpp"
#include "src/autoload.cpp"
#include "src/scene_manager.cpp"
#include "src/renderer.cpp"
#include "src/asset_manager.cpp"

#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL_main.h>

#include <SDL3/SDL.h>

SDL_AppResult SDL_AppInit(void **appstate, int, char **)
{
    if (!SDL_SetAppMetadata("void", "1.0", "com.example.void"))
    {
        SDL_Log("Failed to set app metadata: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMEPAD))
    {
        SDL_Log("Failed to initialized SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    lum::Engine *engine = &lum::Engine::Get();

    if (!engine->Init())
    {
        SDL_Log("Failed to initialized engine");
        return SDL_APP_FAILURE;
    }

    *appstate = engine;

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate)
{
    lum::Engine *engine = static_cast<lum::Engine *>(appstate);

    engine->Update();

    if (!engine->Render())
        return SDL_APP_FAILURE;

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
    lum::Engine *engine = static_cast<lum::Engine *>(appstate);

    engine->Input(event);

    if (event->type == SDL_EVENT_QUIT || event->type == SDL_SCANCODE_ESCAPE)
        return SDL_APP_SUCCESS;

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult)
{
    lum::Engine *engine = static_cast<lum::Engine *>(appstate);

    engine->Shutdown();

    SDL_Quit();
}