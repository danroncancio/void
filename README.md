# Void Engine

Void Engine is a game engine designed for developers who want a clean, efficient, and highly customizable foundation for their projects. Written in C++, it emphasizes a modular, component-based architecture that allows for easy extension and maintenance. By leveraging powerful third-party libraries like SDL, ImGui, and GLM, Void Engine provides the essential tools for building small to medium-sized games without unnecessary overhead.

## Core Features

-   **Modular Architecture**: Core systems like rendering, audio, and assets are decoupled, allowing you to use only what you need.
-   **Component-Based Design**: Build complex game objects by composing small, reusable components.
-   **Custom Rendering Pipeline**: A flexible rendering system with support for custom shaders.
-   **Scene Management**: Easily create, manage, and transition between different game scenes and levels.
-   **Developer Tools**: Integrated ImGui for powerful debugging and in-game tool development.
-   **Asset Management**: A simple yet effective system for loading and managing game assets.

## Project Structure

The repository is organized into three main directories, separating the engine core, game-specific code, and third-party dependencies.

```
void-engine/
├── engine/          # Core engine modules (Renderer, Audio, etc.)
│   ├── include/
│   └── src/
├── game/            # Game-specific logic
│   ├── actors/      # Player, enemies, and other game entities
│   ├── components/  # Sprites, animations, custom scripts
│   ├── scenes/      # Game levels and test environments
│   └── assets/      # Shaders, textures, and other game assets
└── vendor/          # Third-party libraries (as git submodules)
```

## Getting Started

This project uses CMake to generate build files. Follow the steps below to compile and run the engine.

### Prerequisites

-   A C++ compiler (GCC, Clang, MSVC)
-   [CMake](https://cmake.org/download/) 3.15 or later

### Build Instructions

1.  **Clone the repository:**
    ```bash
    git clone https://github.com/yourusername/void-engine.git
    cd void-engine
    ```

2.  **Initialize and update Git submodules:**
    The engine depends on third-party libraries managed as submodules.
    ```bash
    git submodule update --init --recursive
    ```

3.  **Configure the project with CMake:**
    Create a build directory and run CMake from there.
    ```bash
    mkdir build
    cd build
    cmake ..
    ```

4.  **Build the project:**
    Use CMake's build command to compile the engine and game.
    ```bash
    cmake --build .
    ```
    The final executable will be located in the `build/game` directory.

## Third-Party Libraries

Void Engine stands on the shoulders of giants. It integrates the following libraries:

-   [**SDL**](https://www.libsdl.org/): For windowing, input, and audio abstraction.
-   [**ImGui**](https://github.com/ocornut/imgui): For creating debug UIs and developer tools.
-   [**GLM**](https://glm.g-truc.net/): For mathematics common in graphics programming.

## License

Distributed under the MIT License. See `LICENSE` for more information.
