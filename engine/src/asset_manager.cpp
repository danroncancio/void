#include "asset_manager.hpp"

#include "utilities.hpp"

namespace lum
{
    AssetManager::AssetManager() = default;

    AssetManager::~AssetManager() = default;

    bool AssetManager::Init()
    {
        const char *baseDir = SDL_GetBasePath();
        m_assetsDirectoryPath = std::string(baseDir) + "assets/";

        return true;
    }

    void AssetManager::Shutdown()
    {
        auto &gpuDevice = Engine::Get().renderer.gpuDevice;

        // Release shaders

        for (const auto &[_, shader] : m_shaderStorage)
        {
            SDL_ReleaseGPUShader(gpuDevice, shader.data);
        }
    }

    Shader *AssetManager::GetShader(const char *p_tag)
    {
        auto it = m_shaderStorage.find(utils::HashStr32(p_tag));
        if (it == m_shaderStorage.end())
        {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to get shader %s from storage", p_tag);
            return nullptr;
        }

        return &it->second;
    }

    bool AssetManager::LoadShader(const char *p_tag, const char *p_path, bool p_reload)
    {
        auto &gpuDevice = Engine::Get().renderer.gpuDevice;

        std::string glslFullPath = m_assetsDirectoryPath + p_path + ".glsl";
        std::string spvFullPath = m_assetsDirectoryPath + p_path + ".spv";

        size_t shaderSize;
        void *shaderCode = SDL_LoadFile(spvFullPath.c_str(), &shaderSize);
        if (!shaderCode)
        {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed reading shader file: %s", SDL_GetError());
            return false;
        }

        // Used for getting the last modify time of the GLSL file
        SDL_PathInfo pathInfo{};
        SDL_GetPathInfo(glslFullPath.c_str(), &pathInfo);

        SDL_ShaderCross_SPIRV_Info shaderInfo{};
        shaderInfo.name = p_tag;
        shaderInfo.entrypoint = "main";
        shaderInfo.enable_debug = true;
        shaderInfo.bytecode = static_cast<uint8_t *>(shaderCode);
        shaderInfo.bytecode_size = static_cast<size_t>(shaderSize);

        ShaderType type;
        if (SDL_strstr(p_path, ".vert"))
        {
            type = ShaderType::VERTEX;
            shaderInfo.shader_stage = SDL_SHADERCROSS_SHADERSTAGE_VERTEX;
        }
        else if (SDL_strstr(p_path, ".frag"))
        {
            type = ShaderType::FRAGMENT;
            shaderInfo.shader_stage = SDL_SHADERCROSS_SHADERSTAGE_FRAGMENT;
        }
        else
        {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Invalid shader stage loaded");
            return false;
        }

        SDL_ShaderCross_GraphicsShaderMetadata vertMeta{};
        SDL_GPUShader *shader = SDL_ShaderCross_CompileGraphicsShaderFromSPIRV(gpuDevice, &shaderInfo, &vertMeta);
        if (!shader)
        {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to create shader: %s", SDL_GetError());
            SDL_free(shaderCode);
            return false;
        }

        const uint32_t tagHash = utils::HashStr32(p_tag);
        if (!p_reload)
        {
            if (m_shaderStorage.find(tagHash) != m_shaderStorage.end())
            {
                SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "AssetMgr: Overwriting existing shader with tag: %s",
                    p_tag);
            }
            m_shaderStorage.emplace(tagHash, Shader{ p_tag, type, shader, p_path, pathInfo.modify_time });
        }
        else
        {
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "AssetMgr: Shader with tag '%s' is being reloaded", p_tag);
            SDL_ReleaseGPUShader(gpuDevice, m_shaderStorage[tagHash].data);
            m_shaderStorage[tagHash] = Shader{ p_tag, type, shader, p_path, pathInfo.modify_time };
        }

        SDL_free(shaderCode);

        return true;
    }
}