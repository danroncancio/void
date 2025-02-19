cmake_minimum_required (VERSION 3.12)
project ("void")

add_executable (void engine/main.cpp)

add_subdirectory(vendor/glm)

set(SDL_SHARED OFF)
set(SDL_STATIC ON)
add_subdirectory(vendor/sdl)

set(SDLSHADERCROSS_VENDORED ON)
set(SDLSHADERCROSS_DXC OFF)
set(SDLSHADERCROSS_SPIRVCROSS_SHARED OFF)
set(SDLSHADERCROSS_STATIC ON)
set(SDLSHADERCROSS_CLI OFF)
add_subdirectory(vendor/sdl_shadercross)

target_include_directories(void PRIVATE
    engine/include
    vendor/glm
    vendor/sdl/include
    vendor/sdl_shadercross/include
)

target_link_libraries(void PRIVATE
    glm::glm
    SDL3::SDL3-static
    SDL3_shadercross::SDL3_shadercross-static
)

set_property(TARGET void PROPERTY CXX_STANDARD 17)
