#include "renderer.hpp"

#include <array>
#include <algorithm>

#include <glm/gtc/matrix_transform.hpp>
#include <SDL3_shadercross/SDL_shadercross.h>
#include <imgui.h>
#include <implot.h>
#include <backends/imgui_impl_sdl3.h>
#include <backends/imgui_impl_sdlgpu3.h>

#include "utilities.hpp"

namespace lum
{
    Renderer::Renderer() = default;

    Renderer::~Renderer() = default;

    bool Renderer::Init()
    {
        windowDesc.title = "void";
        windowDesc.size = vec2(1280, 720);
        windowDesc.resolution = vec2(240, 360);

        if (!CreateWindowAndGPUDevice())
            return false;

        auto &assetManager = Engine::Get().assetManager;

        // Load shaders

        assetManager.LoadShader("color_quad_frag", "shaders/color_quad.frag");
        assetManager.LoadShader("texture_quad_vert", "shaders/texture_quad.vert");
        assetManager.LoadShader("texture_quad_frag", "shaders/texture_quad.frag");

        // Create graphics pipeline

        CreateGraphicsPipeline("color_quad", "texture_quad_vert", "color_quad_frag");
        CreateGraphicsPipeline("texture_quad", "texture_quad_vert", "texture_quad_frag");

        // Create texture sampler

        if (!SetupRenderTarget())
        {
            SDL_LogError(SDL_LOG_CATEGORY_VIDEO, "Failed to setup render target texture");
            return false;
        }

        if (!SetupQuadData())
        {
            SDL_LogError(SDL_LOG_CATEGORY_VIDEO, "Failed to  setup quad data");
            return false;
        }

        if (!SetupRenderTargetSampler())
        {
            return false;
        }

        ImGuiInit();

        //

        m_windowViewport = { 0.0f, 0.0f, windowDesc.size.x, windowDesc.size.y, 0.0f, 0.0f };

        m_projMat = glm::ortho(0.0f, windowDesc.resolution.x, 0.0f, windowDesc.resolution.y, -1.0f, 1.0f);

        CalculateRenderTargetResolution();

        return true;
    }

    void Renderer::Shutdown()
    {
        ImGuiShutdown();

        SDL_ReleaseGPUTexture(gpuDevice, m_rtTexture);

        SDL_ReleaseGPUSampler(gpuDevice, m_rtSampler);

        SDL_ReleaseGPUBuffer(gpuDevice, m_rtVertexBuffer);
        SDL_ReleaseGPUBuffer(gpuDevice, m_rtIndexBuffer);
        SDL_ReleaseGPUBuffer(gpuDevice, m_quadVertexBuffer);
        SDL_ReleaseGPUBuffer(gpuDevice, m_quadIndexBuffer);

        for (const auto &[_, pipelineInfo] : m_graphicsPipelines)
        {
            SDL_ReleaseGPUGraphicsPipeline(gpuDevice, pipelineInfo.pipeline);
        }

        SDL_DestroyGPUDevice(gpuDevice);

        SDL_DestroyWindow(m_window);
    }

    void Renderer::ToggleWindowFullscreen()
    {
        m_windowFullscreen = !m_windowFullscreen;

        SDL_SetWindowFullscreen(m_window, m_windowFullscreen);
    }

    void Renderer::UpdateWindowSize(uint32_t p_width, uint32_t p_height)
    {
        windowDesc.size = vec2(p_width, p_height);

        CalculateRenderTargetResolution();
    }

    void Renderer::PreRender()
    {
        currentPipelineBinded = nullptr;

        // Start the Dear ImGui frame

        ImGui_ImplSDLGPU3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();
    }

    bool Renderer::RenderFrame()
    {
        ImGui::Render();
        ImDrawData *draw_data = ImGui::GetDrawData();

        m_commandBuffer = SDL_AcquireGPUCommandBuffer(gpuDevice);
        if (!m_commandBuffer)
        {
            SDL_LogError(SDL_LOG_CATEGORY_GPU, "Failed to acquire gpu command buffer: %s", SDL_GetError());
            return false;
        }

        SDL_GPUTexture *swapchainTexture;
        if (!SDL_WaitAndAcquireGPUSwapchainTexture(m_commandBuffer, m_window, &swapchainTexture, nullptr, nullptr))
        {
            SDL_LogError(SDL_LOG_CATEGORY_GPU, "Failed to acquire gpu swapchain texture: %s", SDL_GetError());
            return false;
        }

        if (swapchainTexture && m_rtTexture)
        {
            Imgui_ImplSDLGPU3_PrepareDrawData(draw_data, m_commandBuffer);

            //
            // Draw to render target
            //

            SDL_GPUColorTargetInfo colorTI{};
            colorTI.texture = m_rtTexture;
            colorTI.clear_color = SDL_FColor{ clearColor.x, clearColor.y, clearColor.z, clearColor.w };
            colorTI.load_op = SDL_GPU_LOADOP_CLEAR;
            colorTI.store_op = SDL_GPU_STOREOP_STORE;

            m_renderPass = SDL_BeginGPURenderPass(m_commandBuffer, &colorTI, 1, nullptr);

            // Sort draw queue by layer

            std::sort(m_frameDrawQueue.begin(), m_frameDrawQueue.end(), [](const auto &a, const auto &b) {
                return std::visit([](const auto &arg) { return arg.properties.layer; }, a.second) < 
                       std::visit([](const auto &arg) { return arg.properties.layer; }, b.second);
            });

            // Drawing

            for (const auto &drawDesc : m_frameDrawQueue)
            {
                std::visit([&](auto &&arg) {
                    using T = std::decay_t<decltype(arg)>;

                    if constexpr (std::is_same_v<T, cSprite>)
                        DrawSprite(drawDesc.first, arg);

                }, drawDesc.second);
            }

            SDL_EndGPURenderPass(m_renderPass);

            m_frameDrawQueue.clear();

            //
            // Draw render target to window
            //

            colorTI.texture = swapchainTexture;
            colorTI.clear_color = SDL_FColor{ 0.05f, 0.65f, 0.65f, 1.0f };

            m_renderPass = SDL_BeginGPURenderPass(m_commandBuffer, &colorTI, 1, nullptr);

            SDL_BindGPUGraphicsPipeline(m_renderPass, m_graphicsPipelines[utils::HashStr32("texture_quad")].pipeline);
            SDL_SetGPUViewport(m_renderPass, &m_windowViewport);

            SDL_GPUBufferBinding vertexBinding = { m_rtVertexBuffer, 0 };
            SDL_BindGPUVertexBuffers(m_renderPass, 0, &vertexBinding, 1);

            SDL_GPUBufferBinding indexBinding = { m_rtIndexBuffer, 0 };
            SDL_BindGPUIndexBuffer(m_renderPass, &indexBinding, SDL_GPU_INDEXELEMENTSIZE_16BIT);

            SDL_GPUTextureSamplerBinding samplerBinding{ m_rtTexture, m_rtSampler };
            SDL_BindGPUFragmentSamplers(m_renderPass, 0, &samplerBinding, 1);

            ProjMatUniform projMatUni = { mat4(1.0f), mat4(1.0f), m_projMat };
            SDL_PushGPUVertexUniformData(m_commandBuffer, 0, &projMatUni, sizeof(ProjMatUniform));

            TimeColorUniform timeColUni = { 0.0f, vec4(1.0f) };
            SDL_PushGPUFragmentUniformData(m_commandBuffer, 0, &timeColUni, sizeof(TimeColorUniform));

            SDL_DrawGPUIndexedPrimitives(m_renderPass, 6, 1, 0, 0, 0);

            ImGui_ImplSDLGPU3_RenderDrawData(draw_data, m_commandBuffer, m_renderPass);

            SDL_EndGPURenderPass(m_renderPass);
        }

        if (!SDL_SubmitGPUCommandBuffer(m_commandBuffer))
        {
            SDL_LogError(SDL_LOG_CATEGORY_GPU, "Failed to submit a gpu command buffer: %s", SDL_GetError());
            return false;
        }

        return true;
    }

    void Renderer::AddToDrawQueue(DrawDesc &p_drawDesc)
    {
        m_frameDrawQueue.push_back(p_drawDesc);
    }

    void Renderer::DrawSprite(const cTranslation &p_translationDesc, const cSprite &p_spriteDesc)
    {
        Texture *texture = Engine::Get().assetManager.GetTexture(p_spriteDesc.tag);

        if (currentPipelineBinded != m_graphicsPipelines[utils::HashStr32("texture_quad")].pipeline)
        {
            SDL_BindGPUGraphicsPipeline(m_renderPass, m_graphicsPipelines[utils::HashStr32("texture_quad")].pipeline);

            currentPipelineBinded = m_graphicsPipelines[utils::HashStr32("texture_quad")].pipeline;
        }

        SDL_GPUBufferBinding vertBufferBinding{ m_quadVertexBuffer, 0 };
        SDL_BindGPUVertexBuffers(m_renderPass, 0, &vertBufferBinding, 1);

        SDL_GPUBufferBinding idxBufferBinding{ m_quadIndexBuffer, 0 };
        SDL_BindGPUIndexBuffer(m_renderPass, &idxBufferBinding, SDL_GPU_INDEXELEMENTSIZE_16BIT);

        SDL_GPUTextureSamplerBinding texSamplerBinding{ texture->data, m_rtSampler };
        SDL_BindGPUFragmentSamplers(m_renderPass, 0, &texSamplerBinding, 1);

        m_modelMat = mat4(1.0f);
        m_modelMat = glm::translate(m_modelMat, vec3(p_translationDesc.position, 0.0f));
        m_modelMat = glm::rotate(m_modelMat, radians(p_translationDesc.rotation), vec3(0.0f, 0.0f, 1.0f));
        m_modelMat = glm::scale(m_modelMat, vec3(texture->size.x * p_translationDesc.scale, texture->size.y * p_translationDesc.scale, 1.0f));

        ProjMatUniform compoundMat{ m_modelMat, m_viewMat, m_projMat };
        SDL_PushGPUVertexUniformData(m_commandBuffer, 0, &compoundMat, sizeof(ProjMatUniform));

        TimeColorUniform timeColUni = { 0.0f, p_spriteDesc.properties.modulateColor };
        SDL_PushGPUFragmentUniformData(m_commandBuffer, 0, &timeColUni, sizeof(TimeColorUniform));

        SDL_DrawGPUIndexedPrimitives(m_renderPass, 6, 1, 0, 0, 0);
    }

    bool Renderer::CreateWindowAndGPUDevice()
    {
        m_window = SDL_CreateWindow(windowDesc.title, static_cast<int>(windowDesc.size.x), static_cast<int>(windowDesc.size.y), SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY);
        if (!m_window)
        {
            SDL_LogError(SDL_LOG_CATEGORY_VIDEO, "Failed to create SDL window: %s", SDL_GetError());
            return false;
        }

        gpuDevice = SDL_CreateGPUDevice(SDL_ShaderCross_GetSPIRVShaderFormats(), true, nullptr);
        if (!gpuDevice)
        {
            SDL_LogError(SDL_LOG_CATEGORY_VIDEO, "Failed to create SDL GPU device: %s", SDL_GetError());
            return false;
        }

        if (!SDL_ClaimWindowForGPUDevice(gpuDevice, m_window))
        {
            SDL_LogError(SDL_LOG_CATEGORY_VIDEO, "Failed to claim window for GPU device: %s", SDL_GetError());
            return false;
        }

        SDL_SetGPUSwapchainParameters(gpuDevice, m_window, SDL_GPU_SWAPCHAINCOMPOSITION_SDR, SDL_GPU_PRESENTMODE_VSYNC);

        return true;
    }

    bool Renderer::CreateGraphicsPipeline(const char *p_tag, const char *p_vertTag, const char *p_fragTag, bool p_reload)
    {
        // Blend state for color target

        SDL_GPUColorTargetBlendState blendState{};
        blendState.enable_blend = true;
        blendState.alpha_blend_op = SDL_GPU_BLENDOP_ADD;
        blendState.color_blend_op = SDL_GPU_BLENDOP_ADD;
        blendState.src_color_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA;
        blendState.src_alpha_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA;
        blendState.dst_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
        blendState.dst_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;

        // Screen texture format

        SDL_GPUTextureFormat screenTextureFormat = SDL_GetGPUSwapchainTextureFormat(gpuDevice, m_window);

        // Build color target description struct

        SDL_GPUColorTargetDescription colorTargetDesc{};
        colorTargetDesc.format = screenTextureFormat;
        colorTargetDesc.blend_state = blendState;

        std::array<SDL_GPUColorTargetDescription, 1> colorTargetDescs = { colorTargetDesc };

        // Vertex buffer description

        SDL_GPUVertexBufferDescription vertBufferDesc{};
        vertBufferDesc.slot = 0;
        vertBufferDesc.pitch = sizeof(PosTexVertex);
        vertBufferDesc.input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX;
        vertBufferDesc.instance_step_rate = 0;

        std::array<SDL_GPUVertexBufferDescription, 1> vertBufferDescs = { vertBufferDesc };

        // Vertex attributes

        std::array<SDL_GPUVertexAttribute, 2> vertAttributes = {
            SDL_GPUVertexAttribute{0, 0, SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3, 0},
            SDL_GPUVertexAttribute{1, 0, SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2, sizeof(vec3)}
        };

        // Pipeline target info

        SDL_GPUGraphicsPipelineTargetInfo pipelineTI{};
        pipelineTI.num_color_targets = 1;
        pipelineTI.color_target_descriptions = colorTargetDescs.data();

        // Vertex input state

        SDL_GPUVertexInputState vertIS{};
        vertIS.num_vertex_buffers = 1;
        vertIS.vertex_buffer_descriptions = vertBufferDescs.data();
        vertIS.num_vertex_attributes = 2;
        vertIS.vertex_attributes = vertAttributes.data();

        // GPU rasterizer state

        SDL_GPURasterizerState gpuRS{};
        gpuRS.fill_mode = SDL_GPU_FILLMODE_FILL;
        gpuRS.cull_mode = SDL_GPU_CULLMODE_BACK;
        gpuRS.front_face = SDL_GPU_FRONTFACE_CLOCKWISE;

        // Pipeline create info

        auto &assetManager = Engine::Get().assetManager;

        SDL_GPUGraphicsPipelineCreateInfo pipelineCI{};
        pipelineCI.vertex_shader = assetManager.GetShader(p_vertTag)->data;
        pipelineCI.fragment_shader = assetManager.GetShader(p_fragTag)->data;
        pipelineCI.vertex_input_state = vertIS;
        pipelineCI.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;
        pipelineCI.rasterizer_state = gpuRS;
        // pipelineCI.multisample_state
        // pipelineCI.depth_stencil_state
        pipelineCI.target_info = pipelineTI;

        SDL_GPUGraphicsPipeline *pipeline = SDL_CreateGPUGraphicsPipeline(gpuDevice, &pipelineCI);
        if (!pipeline)
        {
            SDL_LogError(SDL_LOG_CATEGORY_VIDEO, "Failed to create a graphics pipeline SDL_CreateGPUGraphicsPipeline: %s", SDL_GetError());
            return false;
        }

        if (!p_reload)
        {
            m_graphicsPipelines.emplace(utils::HashStr32(p_tag), GraphicPipelineInfo{ pipeline, p_vertTag, p_fragTag });
        }
        else
        {
            SDL_LogInfo(SDL_LOG_CATEGORY_VIDEO, "Renderer: Graphics pipeline with tag '%s' is being reloaded", p_tag);
            m_graphicsPipelines[utils::HashStr32(p_tag)] = GraphicPipelineInfo{ pipeline, p_vertTag, p_fragTag };
        }

        return true;
    }

    SDL_GPUBuffer *Renderer::CreateGPUBuffer(SDL_GPUBufferUsageFlags p_usage, uint32_t p_size, const char *p_debugName) const
    {
        SDL_GPUBufferCreateInfo bufferCI{};
        bufferCI.usage = p_usage;
        bufferCI.size = p_size;

        auto buffer = SDL_CreateGPUBuffer(gpuDevice, &bufferCI);
        if (!buffer)
        {
            SDL_LogError(SDL_LOG_CATEGORY_GPU, "Failed to create GPU buffer: %s", SDL_GetError());
            return nullptr;
        }
        SDL_SetGPUBufferName(gpuDevice, buffer, p_debugName);

        return buffer;
    }

    bool Renderer::SetupRenderTarget()
    {
        // Create the render target texture

        SDL_GPUTextureCreateInfo texInfo{};
        texInfo.type = SDL_GPU_TEXTURETYPE_2D;
        texInfo.format = SDL_GPU_TEXTUREFORMAT_B8G8R8A8_UNORM;
        texInfo.usage = SDL_GPU_TEXTUREUSAGE_SAMPLER | SDL_GPU_TEXTUREUSAGE_COLOR_TARGET;
        texInfo.width = static_cast<uint32_t>(windowDesc.resolution.x);
        texInfo.height = static_cast<uint32_t>(windowDesc.resolution.y);
        texInfo.layer_count_or_depth = 1;
        texInfo.num_levels = 1;
        texInfo.sample_count = SDL_GPU_SAMPLECOUNT_1;

        m_rtTexture = SDL_CreateGPUTexture(gpuDevice, &texInfo);
        if (!m_rtTexture)
        {
            SDL_LogError(SDL_LOG_CATEGORY_GPU, "Failed to create render target texture: %s", SDL_GetError());
            return false;
        }

        // Create transfer buffer
        // TODO: Maybe try to abstract all this shit?

        SDL_GPUTransferBufferCreateInfo transferCI{};
        transferCI.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
        transferCI.size = (sizeof(PosTexVertex) * 4) + (sizeof(uint16_t) * 6); // Four vertices and six indices per quad
        SDL_GPUTransferBuffer *transferBuffer = SDL_CreateGPUTransferBuffer(gpuDevice, &transferCI);
        PosTexVertex *transferData = static_cast<PosTexVertex *>(SDL_MapGPUTransferBuffer(gpuDevice, transferBuffer, false));

        // Map data

        vec2 origin = vec2(static_cast<int>(windowDesc.resolution.x) >> 1,
            static_cast<int>(windowDesc.resolution.y) >> 1); // Middle of the texture

        mat4 m_modelMat = mat4(1.0f);
        m_modelMat = glm::translate(m_modelMat, vec3(vec2(0.0f) + origin, 0.0f));
        m_modelMat = glm::scale(m_modelMat, vec3(windowDesc.resolution.x, windowDesc.resolution.y, 1.0f));

        transferData[0] = PosTexVertex{ m_modelMat * vec4(-0.5f, 0.5f, 0.0f, 1.0f), vec2(0.0f, 0.0f) };
        transferData[1] = PosTexVertex{ m_modelMat * vec4(0.5f, 0.5f, 0.0f, 1.0f), vec2(1.0f, 0.0f) };
        transferData[2] = PosTexVertex{ m_modelMat * vec4(0.5f, -0.5f, 0.0f, 1.0f), vec2(1.0f, 1.0f) };
        transferData[3] = PosTexVertex{ m_modelMat * vec4(-0.5f, -0.5f, 0.0f, 1.0f), vec2(0.0f, 1.0f) };

        uint16_t *indexData = reinterpret_cast<uint16_t *>(&transferData[4]);
        indexData[0] = 0;
        indexData[1] = 1;
        indexData[2] = 2;
        indexData[3] = 0;
        indexData[4] = 2;
        indexData[5] = 3;

        SDL_UnmapGPUTransferBuffer(gpuDevice, transferBuffer);

        // Upload data to GPU with a copy pass

        SDL_GPUCommandBuffer *uploadCmdBuffer = SDL_AcquireGPUCommandBuffer(gpuDevice);
        if (!uploadCmdBuffer)
        {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to acquire upload command buffer: %s", SDL_GetError());
            return false;
        }

        // Create vertex and index buffers

        m_rtVertexBuffer = CreateGPUBuffer(SDL_GPU_BUFFERUSAGE_VERTEX, sizeof(PosTexVertex) * 4, "rendertarget_vertex_buffer");
        m_rtIndexBuffer = CreateGPUBuffer(SDL_GPU_BUFFERUSAGE_INDEX, sizeof(uint16_t) * 6, "rendertarget_index_buffer");

        if (!m_rtVertexBuffer || !m_rtIndexBuffer)
            return false;

        SDL_GPUCopyPass *copyPass = SDL_BeginGPUCopyPass(uploadCmdBuffer);

        SDL_GPUTransferBufferLocation vertBufferLoc{};
        vertBufferLoc.transfer_buffer = transferBuffer;
        vertBufferLoc.offset = 0;

        SDL_GPUBufferRegion vertBufferReg{};
        vertBufferReg.buffer = m_rtVertexBuffer;
        vertBufferReg.offset = 0;
        vertBufferReg.size = sizeof(PosTexVertex) * 4;

        SDL_UploadToGPUBuffer(copyPass, &vertBufferLoc, &vertBufferReg, false);

        SDL_GPUTransferBufferLocation idxBufferLoc{};
        idxBufferLoc.transfer_buffer = transferBuffer;
        idxBufferLoc.offset = sizeof(PosTexVertex) * 4;

        SDL_GPUBufferRegion idxBufferReg{};
        idxBufferReg.buffer = m_rtIndexBuffer;
        idxBufferReg.offset = 0;
        idxBufferReg.size = sizeof(uint16_t) * 6;

        SDL_UploadToGPUBuffer(copyPass, &idxBufferLoc, &idxBufferReg, false);

        SDL_EndGPUCopyPass(copyPass);
        SDL_SubmitGPUCommandBuffer(uploadCmdBuffer);

        SDL_ReleaseGPUTransferBuffer(gpuDevice, transferBuffer);


        return true;
    }

    bool Renderer::SetupQuadData()
    {
        // Create transfer buffer and mapped vertex and index data to it

        SDL_GPUTransferBufferCreateInfo transferCI{};
        transferCI.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
        transferCI.size = (sizeof(PosTexVertex) * 4) + (sizeof(uint16_t) * 6);
        SDL_GPUTransferBuffer *transferBuffer = SDL_CreateGPUTransferBuffer(gpuDevice, &transferCI);

        PosTexVertex *mappedTransferData = static_cast<PosTexVertex *>(SDL_MapGPUTransferBuffer(gpuDevice, transferBuffer, false));

        mappedTransferData[0] = PosTexVertex{ vec3(-0.5f, 0.5f, 0.0f), vec2(0.0f, 0.0f) };
        mappedTransferData[1] = PosTexVertex{ vec3(0.5f, 0.5f, 0.0f), vec2(1.0f, 0.0f) };
        mappedTransferData[2] = PosTexVertex{ vec3(0.5f, -0.5f, 0.0f), vec2(1.0f, 1.0f) };
        mappedTransferData[3] = PosTexVertex{ vec3(-0.5f, -0.5f, 0.0f), vec2(0.0f, 1.0f) };

        uint16_t *indexData = reinterpret_cast<uint16_t *>(&mappedTransferData[4]);
        indexData[0] = 0;
        indexData[1] = 1;
        indexData[2] = 2;
        indexData[3] = 0;
        indexData[4] = 2;
        indexData[5] = 3;

        SDL_UnmapGPUTransferBuffer(gpuDevice, transferBuffer);

        // Upload transfer buffer data to GPU using a copy pass

        SDL_GPUCommandBuffer *uploadCmdBuffer = SDL_AcquireGPUCommandBuffer(gpuDevice);
        if (!uploadCmdBuffer)
        {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to acquire upload command buffer: %s", SDL_GetError());
            return false;
        }

        m_quadVertexBuffer = CreateGPUBuffer(SDL_GPU_BUFFERUSAGE_VERTEX, sizeof(PosTexVertex) * 4, "quad_vertex_buffer");
        m_quadIndexBuffer = CreateGPUBuffer(SDL_GPU_BUFFERUSAGE_INDEX, sizeof(uint16_t) * 6, "quad_index_buffer");

        if (!m_quadVertexBuffer || !m_quadIndexBuffer)
            return false;

        SDL_GPUCopyPass *copyPass = SDL_BeginGPUCopyPass(uploadCmdBuffer);

        // Vertex data

        SDL_GPUTransferBufferLocation vertBufferLoc{};
        vertBufferLoc.transfer_buffer = transferBuffer;
        vertBufferLoc.offset = 0;

        SDL_GPUBufferRegion vertBufferReg{};
        vertBufferReg.buffer = m_quadVertexBuffer;
        vertBufferReg.offset = 0;
        vertBufferReg.size = sizeof(PosTexVertex) * 4;

        SDL_UploadToGPUBuffer(copyPass, &vertBufferLoc, &vertBufferReg, false);

        // Index data

        SDL_GPUTransferBufferLocation idxBufferLoc{};
        idxBufferLoc.transfer_buffer = transferBuffer;
        idxBufferLoc.offset = sizeof(PosTexVertex) * 4;

        SDL_GPUBufferRegion idxBufferReg{};
        idxBufferReg.buffer = m_quadIndexBuffer;
        idxBufferReg.offset = 0;
        idxBufferReg.size = sizeof(uint16_t) * 6;

        SDL_UploadToGPUBuffer(copyPass, &idxBufferLoc, &idxBufferReg, false);

        SDL_EndGPUCopyPass(copyPass);
        SDL_SubmitGPUCommandBuffer(uploadCmdBuffer);

        SDL_ReleaseGPUTransferBuffer(gpuDevice, transferBuffer);

        return true;
    }

    bool Renderer::SetupRenderTargetSampler()
    {
        // Sampler with nearest neighbour filtering and without repeating
        // Good for pixel art

        SDL_GPUSamplerCreateInfo samplerCI{};
        samplerCI.min_filter = SDL_GPU_FILTER_NEAREST;
        samplerCI.mag_filter = SDL_GPU_FILTER_NEAREST;
        samplerCI.mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_NEAREST;
        samplerCI.address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
        samplerCI.address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
        samplerCI.address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;

        m_rtSampler = SDL_CreateGPUSampler(gpuDevice, &samplerCI);
        if (!m_rtSampler)
        {
            SDL_LogError(SDL_LOG_CATEGORY_GPU, "Failed to create a texture sampler: %s", SDL_GetError());
            return false;
        }

        return true;
    }

    void Renderer::CalculateRenderTargetResolution()
    {
        auto winSize = windowDesc.size;
        auto resolution = windowDesc.resolution;

        // Calculate the scaling factors based on internal resolution
        float widthRatio = static_cast<float>(winSize.x) / resolution.x;
        float heightRatio = static_cast<float>(winSize.y) / resolution.y;

        // Use the floor of the ratios to determine integer scaling factors
        int widthScale = static_cast<int>(SDL_floorf(widthRatio));
        int heightScale = static_cast<int>(SDL_floorf(heightRatio));

        // Choose the minimum scale to ensure we stay within the limits
        int scale = std::min(widthScale, heightScale);

        // Calculate the new window dimensions using integer scaling
        int newWinWidth = static_cast<int>(resolution.x) * scale;
        int newWinHeight = static_cast<int>(resolution.y) * scale;

        windowDesc.upscaledResolution.x = static_cast<float>(newWinWidth);
        windowDesc.upscaledResolution.y = static_cast<float>(newWinHeight);

        windowDesc.upscaledResolution.x = SDL_max(windowDesc.upscaledResolution.x, windowDesc.resolution.x);
        windowDesc.upscaledResolution.y = SDL_max(windowDesc.upscaledResolution.y, windowDesc.resolution.y);

        // Calculate viewport position offset
        windowDesc.targetOffset.x = SDL_floorf((static_cast<int>(winSize.x) - windowDesc.upscaledResolution.x) / 2);
        windowDesc.targetOffset.y = SDL_floorf((static_cast<int>(winSize.y) - windowDesc.upscaledResolution.y) / 2);

        windowDesc.targetOffset.x = SDL_max(windowDesc.targetOffset.x, 0);
        windowDesc.targetOffset.y = SDL_max(windowDesc.targetOffset.y, 0);

        m_windowViewport = {
            static_cast<float>(windowDesc.targetOffset.x),
            static_cast<float>(windowDesc.targetOffset.y),
            static_cast<float>(windowDesc.upscaledResolution.x),
            static_cast<float>(windowDesc.upscaledResolution.y),
            0.0f,
            0.0f
        };
    }

    void Renderer::ImGuiInit()
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImPlot::CreateContext();
        ImGuiIO &io = ImGui::GetIO();
        (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

        // Setup Dear ImGui style
        // ImGui::StyleColorsDark();
        ImGui::StyleColorsLight();

        ImGuiStyle &style = ImGui::GetStyle();

        // Rounded corners
        style.WindowRounding = 5.0f;
        style.ChildRounding = 5.0f;
        style.FrameRounding = 2.5f;
        style.PopupRounding = 2.5f;

        // Setup Platform/Renderer backends

        ImGui_ImplSDL3_InitForSDLGPU(m_window);
        ImGui_ImplSDLGPU3_InitInfo init_info = {};
        init_info.Device = gpuDevice;
        init_info.ColorTargetFormat = SDL_GetGPUSwapchainTextureFormat(gpuDevice, m_window);
        init_info.MSAASamples = SDL_GPU_SAMPLECOUNT_1;
        ImGui_ImplSDLGPU3_Init(&init_info);
    }

    void Renderer::ImGuiShutdown()
    {
        ImPlot::DestroyContext();
        ImGui_ImplSDL3_Shutdown();
        ImGui_ImplSDLGPU3_Shutdown();
        ImGui::DestroyContext();
    }
}