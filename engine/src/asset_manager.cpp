#include "asset_manager.hpp"

#include <SDL3_shadercross/SDL_shadercross.h>
#include <SDL3_image/SDL_image.h>
#include <vorbis/vorbisfile.h>

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

    Sound *AssetManager::GetSound(const char *p_tag)
    {
        auto it = m_soundStorage.find(utils::HashStr32(p_tag));
        if (it == m_soundStorage.end())
        {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to get sound %s from storage", p_tag);
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

    bool AssetManager::LoadSound(const char *p_tag, const char *p_path)
    {
        Sound sound;

        std::string fullPath = m_assetsDirectoryPath + p_path;

        if (SDL_strstr(p_path, ".ogg"))
        {
            if (!LoadOGG(fullPath.c_str(), &sound.audioSpec, sound.buffer))
            {
                return false;
            }

            sound.length = static_cast<uint32_t>(sound.buffer.size());
        }
        else if (SDL_strstr(p_path, ".wav"))
        {
            uint8_t *tempBuffer;
            if (!SDL_LoadWAV(fullPath.c_str(), &sound.audioSpec, &tempBuffer, &sound.length))
            {
                SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to load WAV file: %s", SDL_GetError());
                return false;
            }

            sound.buffer.assign(tempBuffer, tempBuffer + sound.length);
            SDL_free(tempBuffer);
        }

        m_soundStorage[utils::HashStr32(p_tag)] = std::move(sound);

        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Sound '%s' loaded from '%s'", p_tag, p_path);

        return true;
    }

    void AssetManager::CheckForModifiedAssets()
    {
        auto &renderer = Engine::Get().renderer;
        SDL_PathInfo pathInfo{};

        // Textures

        for (auto const &[_, textureAsset] : m_textureStorage)
        {
            if (!SDL_GetPathInfo((m_assetsDirectoryPath + textureAsset.filePath).c_str(), &pathInfo))
            {
                SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "AssetMgr: Couldn't find texture asset file while checking for changes");
                continue;
            }

            if (textureAsset.lastModifyTime == pathInfo.modify_time)
                continue;

            SDL_Log("--- Texture asset reload");

            // Even though we are doing this at the beginning of the frame, we need to wait for the GPU to idle
            // maybe because of like concurrency stuff?
            SDL_WaitForGPUIdle(renderer.gpuDevice);

            LoadTexture(textureAsset.tag, textureAsset.filePath, true);
        }

        // Shaders

        for (auto &[tag, shaderAsset] : m_shaderStorage)
        {
            if (!SDL_GetPathInfo((m_assetsDirectoryPath + shaderAsset.filePath + ".glsl").c_str(), &pathInfo))
            {
                SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                    "AssetMgr: Couldn't find shader asset file while checking for changes");
                continue;
            }

            if (shaderAsset.lastModifyTime == pathInfo.modify_time)
                continue;

            SDL_Log("--- Shader asset reload");

            if (!CompileShader(shaderAsset.filePath))
            {
                SDL_GetPathInfo((m_assetsDirectoryPath + shaderAsset.filePath + ".glsl").c_str(), &pathInfo);
                shaderAsset.lastModifyTime = pathInfo.modify_time;
                continue;
            }

            SDL_WaitForGPUIdle(renderer.gpuDevice);
            LoadShader(shaderAsset.tag, shaderAsset.filePath, true);

            // Find pipelines that use the shader just compiled

            for (auto const &[pipelineTag, pipelineDesc] : renderer.m_graphicsPipelines)
            {
                // 1 vert tag | 2 frag tag

                if (SDL_strcmp(shaderAsset.tag, pipelineDesc.vertTag) == 0 || SDL_strcmp(shaderAsset.tag, pipelineDesc.fragTag) == 0)
                {
                    SDL_ReleaseGPUGraphicsPipeline(renderer.gpuDevice, pipelineDesc.pipeline);

                    SDL_Log("Graphics pipeline '%s' will be recreated", pipelineDesc.tag);

                    renderer.CreateGraphicsPipeline(pipelineDesc.tag, pipelineDesc.vertTag, pipelineDesc.fragTag, true);
                }
            }
        }
    }

    bool AssetManager::CompileShader(const char *p_path)
    {
        std::string shaderStage = "-fshader-stage=";

        if (SDL_strstr(p_path, ".vert"))
        {
            shaderStage += "vert";
        }
        else if (SDL_strstr(p_path, ".frag"))
        {
            shaderStage += "frag";
        }

        std::string fullPath = m_assetsDirectoryPath + p_path;
        std::string inputPath = fullPath + ".glsl";
        std::string outputPath = fullPath + ".spv";

        // IMPORTANT: Assuming we have glslc installed (VulkanSDK) and in the PATH

        const char *args[] = { "glslc", shaderStage.c_str(), inputPath.c_str(), "-o", outputPath.c_str(), NULL };
        SDL_Process *shaderCompProc = SDL_CreateProcess(args, false);
        if (!shaderCompProc)
        {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "AssetMgr: Couldn't launch glslc process for compiling shader");
            return false;
        }

        int exitCode;
        SDL_WaitProcess(shaderCompProc, true, &exitCode);

        if (exitCode == 0)
        {
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "AssetMgr: Shader '%s' compiling successful.", p_path);
        }
        else
        {
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "AssetMgr: Error compiling shader '%s'", p_path);
            return false;
        }

        SDL_DestroyProcess(shaderCompProc);

        return true;
    }

    bool AssetManager::LoadOGG(const char *p_path, SDL_AudioSpec *p_spec, std::vector<uint8_t> &p_outBuffer)
    {
        OggVorbis_File vf;
        ov_fopen(p_path, &vf);

        vorbis_info *sound_info = ov_info(&vf, -1);
        p_spec->channels = sound_info->channels;
        p_spec->freq = sound_info->rate;
        p_spec->format = SDL_AUDIO_S16; // 16-bit signed PCM

        int32_t bitstream;
        char    buffer[4096];
        int64_t bytes;
        do
        {
            bytes = ov_read(&vf, buffer, sizeof(buffer), 0, 2, 1, &bitstream);
            if (bytes > 0)
            {
                p_outBuffer.insert(p_outBuffer.end(), buffer, buffer + bytes);
            }
        } while (bytes > 0);

        ov_clear(&vf);

        return true;
    }
}