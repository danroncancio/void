cmake_minimum_required (VERSION 3.12)
project ("void")

add_executable (void engine/main.cpp)

set(SDL_SHARED OFF)
set(SDL_STATIC ON)
add_subdirectory(vendor/sdl)

target_include_directories(void PRIVATE
    engine/include
    vendor/sdl/include
)

target_link_libraries(void PRIVATE
    SDL3::SDL3-static
)

set_property(TARGET void PROPERTY CXX_STANDARD 17)
