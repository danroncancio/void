#include "asset_manager.hpp"

#include <SDL3_shadercross/SDL_shadercross.h>
#include <SDL3_image/SDL_image.h>

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

        // Release textures

        for (const auto &[_, texture] : m_textureStorage)
        {
            SDL_ReleaseGPUTexture(gpuDevice, texture.data);
        }

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
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to get shader %s from storage", p_tag);
            return nullptr;
        }

        return &it->second;
    }

    Texture *AssetManager::GetTexture(const char *p_tag)
    {
        auto it = m_textureStorage.find(utils::HashStr32(p_tag));
        if (it == m_textureStorage.end())
        {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to get texture %s from storage", p_tag);
            return nullptr;
        }

        return &it->second;
    }

    bool AssetManager::LoadShader(const char *p_tag, const char *p_path, bool p_reload)
    {
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

        auto &gpuDevice = Engine::Get().renderer.gpuDevice;

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
                SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Overwriting existing shader with tag: %s",
                    p_tag);
            }
            m_shaderStorage.emplace(tagHash, Shader{ p_tag, type, shader, p_path, pathInfo.modify_time });
        }
        else
        {
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Shader with tag '%s' is being reloaded", p_tag);
            SDL_ReleaseGPUShader(gpuDevice, m_shaderStorage[tagHash].data);
            m_shaderStorage[tagHash] = Shader{ p_tag, type, shader, p_path, pathInfo.modify_time };
        }

        SDL_free(shaderCode);

        return true;
    }

    bool AssetManager::LoadTexture(const char *p_tag, const char *p_path, bool p_reload)
    {
        std::string fullPath = m_assetsDirectoryPath + p_path;

        SDL_Surface *imageData = IMG_Load(fullPath.c_str());
        if (!imageData)
        {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to load image file: %s", SDL_GetError());
            return false;
        }

        if (!p_reload)
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Texture '%s' loaded from '%s' (%dx%d)", p_tag, p_path, imageData->w, imageData->h);

        SDL_PathInfo pathInfo{}; // Used for getting the last modify time of the file
        SDL_GetPathInfo(fullPath.c_str(), &pathInfo);

        // Handle different format images to force them into RGBA 32bit

        if (imageData->format == SDL_PIXELFORMAT_RGB24)
            imageData = SDL_ConvertSurface(imageData, SDL_PIXELFORMAT_RGBA32);

        // Create texture description

        SDL_GPUTextureCreateInfo textureCI{};
        textureCI.type = SDL_GPU_TEXTURETYPE_2D;
        textureCI.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
        textureCI.usage = SDL_GPU_TEXTUREUSAGE_SAMPLER;
        textureCI.width = imageData->w;
        textureCI.height = imageData->h;
        textureCI.layer_count_or_depth = 1;
        textureCI.num_levels = 1;

        // Create texture

        auto &gpuDevice = Engine::Get().renderer.gpuDevice;

        auto texture = SDL_CreateGPUTexture(gpuDevice, &textureCI);
        if (!texture)
        {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "AssetMgr: Failed to create texture: %s", SDL_GetError());
            return false;
        }

        SDL_SetGPUTextureName(gpuDevice, texture, p_tag);

        // Map data

        SDL_GPUTransferBufferCreateInfo texTransferInfo{};
        texTransferInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
        texTransferInfo.size = imageData->w * imageData->h * 4; // RGBA -> 1 byte per channel

        SDL_GPUTransferBuffer *texTransferBuffer = SDL_CreateGPUTransferBuffer(gpuDevice, &texTransferInfo);

        uint8_t *mappedBuffer = static_cast<uint8_t *>(SDL_MapGPUTransferBuffer(gpuDevice, texTransferBuffer, false));

        SDL_memcpy(mappedBuffer, imageData->pixels, imageData->w * imageData->h * 4);

        SDL_UnmapGPUTransferBuffer(gpuDevice, texTransferBuffer);

        // Upload data to GPU with a copy pass

        SDL_GPUCommandBuffer *uploadCmdBuffer = SDL_AcquireGPUCommandBuffer(gpuDevice);
        if (!uploadCmdBuffer)
        {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "AssetMgr: Failed to acquire upload command buffer: %s", SDL_GetError());
            return false;
        }
        SDL_GPUCopyPass *copyPass = SDL_BeginGPUCopyPass(uploadCmdBuffer);

        SDL_GPUTextureTransferInfo texTransInfo{};
        texTransInfo.transfer_buffer = texTransferBuffer;
        texTransInfo.offset = 0;

        SDL_GPUTextureRegion texRegion{};
        texRegion.texture = texture;
        texRegion.w = imageData->w;
        texRegion.h = imageData->h;
        texRegion.d = 1;

        SDL_UploadToGPUTexture(copyPass, &texTransInfo, &texRegion, false);

        SDL_EndGPUCopyPass(copyPass);
        SDL_SubmitGPUCommandBuffer(uploadCmdBuffer);

        // Store texture

        const uint32_t tagHash = utils::HashStr32(p_tag);

        if (!p_reload)
        {
            if (m_textureStorage.find(tagHash) != m_textureStorage.end())
            {
                SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "AssetMgr: Overwriting existing texture with tag: %s", p_tag);
            }
            m_textureStorage.emplace(tagHash, Texture{ p_tag, vec2(imageData->w, imageData->h), texture, p_path, pathInfo.modify_time });
        }
        else
        {
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "AssetMgr: Texture with tag '%s' is being reloaded", p_tag);
            SDL_ReleaseGPUTexture(gpuDevice, m_textureStorage[tagHash].data);
            m_textureStorage[tagHash] = Texture{ p_tag, vec2(imageData->w, imageData->h), texture, p_path, pathInfo.modify_time };
        }

        SDL_DestroySurface(imageData);
        SDL_ReleaseGPUTransferBuffer(gpuDevice, texTransferBuffer);

        return true;
    }
}