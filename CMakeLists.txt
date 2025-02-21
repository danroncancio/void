cmake_minimum_required (VERSION 3.12)
project ("void")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

# Imgui files

file(GLOB IMGUI_FILES Vendor/imgui/*.cpp Vendor/imgui/*.h)
file(GLOB IMGUI_BACKEND_FILES
    Vendor/imgui/backends/imgui_impl_sdl3.h
    Vendor/imgui/backends/imgui_impl_sdl3.cpp
    Vendor/imgui/backends/imgui_impl_sdlgpu3.h
    Vendor/imgui/backends/imgui_impl_sdlgpu3.cpp
    Vendor/imgui/backends/imgui_impl_sdlgpu3_shaders.h
)

# Implot files

file(GLOB IMPLOT_FILES Vendor/implot/*.cpp Vendor/implot/*.h)

add_executable (void
    engine/main.cpp
    ${IMGUI_FILES}
    ${IMGUI_BACKEND_FILES}
    ${IMPLOT_FILES}
)

# Dependencies

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

set(SDLIMAGE_VENDORED OFF)
set(BUILD_SHARED_LIBS OFF)
add_subdirectory(vendor/sdl_image)

set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF)
add_subdirectory(vendor/libogg)

set(OGG_LIBRARY "${CMAKE_BINARY_DIR}/lib")
set(OGG_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/vendor/libvorbis/include")
add_subdirectory(vendor/libvorbis)

target_include_directories(void PRIVATE
    engine/include
    vendor/glm
    vendor/sdl/include
    vendor/sdl_shadercross/include
    vendor/sdl_image/include
    vendor/imgui
    vendor/implot
    vendor/libogg/include
    vendor/libvorbis/include
)

target_link_libraries(void PRIVATE
    glm::glm
    SDL3::SDL3-static
    SDL3_shadercross::SDL3_shadercross-static
    SDL3_image::SDL3_image-static
    ogg
    vorbis
    vorbisfile
)

set_property(TARGET void PROPERTY CXX_STANDARD 17)
