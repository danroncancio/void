#ifndef ASSET_MANAGER_H
#define ASSET_MANAGER_H

#include <string>
#include <unordered_map>

#include "asset_types.hpp"

namespace lum
{
    class AssetManager
    {
    public:
        AssetManager();
        ~AssetManager();

        bool Init();
        void Shutdown();

        Shader *GetShader(const char *p_tag);
        Texture *GetTexture(const char *p_tag);
        Sound *GetSound(const char *p_tag);
        bool LoadShader(const char *p_tag, const char *p_path, bool p_reload = false);
        bool LoadTexture(const char *p_tag, const char *p_path, bool p_reload = false);
        bool LoadSound(const char *p_tag, const char *p_path);
        void CheckForModifiedAssets();

    private:
        std::string m_assetsDirectoryPath{};
        std::unordered_map<uint32_t, Shader> m_shaderStorage{};
        std::unordered_map<uint32_t, Texture> m_textureStorage{};
        std::unordered_map<uint32_t, Sound> m_soundStorage{};

    private:
        bool CompileShader(const char *p_path);
        bool LoadOGG(const char *p_path, SDL_AudioSpec *p_spec, std::vector<uint8_t> &p_outBuffer);
    };
}

#endif // !ASSET_MANAGER_H
