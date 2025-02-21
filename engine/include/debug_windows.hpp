#ifndef DEBUG_WINDOWS_H
#define DEBUG_WINDOWS_H

#include <vector>

#include <imgui.h>
#include <implot.h>

namespace lum::metrics
{
    class MetricsWindows
    {
    public:
        uint32_t fps{};
        float accumulatedTime{};
        float uptime{};
        std::vector<float> frameTimes{};
        uint32_t drawCalls{};
        float renderFrameTime{};
        float updateFrameTime{};

    public:
        MetricsWindows() = default;
        ~MetricsWindows() = default;

        void StatsUpdate(float p_deltaTime)
        {
            frameTimes.push_back(p_deltaTime);
            accumulatedTime += p_deltaTime;

            while (accumulatedTime > 1.0f)
            {
                accumulatedTime -= frameTimes.front();
                frameTimes.erase(frameTimes.begin());
            }

            fps = static_cast<int>(frameTimes.size());
            uptime += p_deltaTime;
        };

        void ShowStatsWindows(float p_deltaTime)
        {
            static float frameTimeGraph[500] = { 0 };
            static int frameIndex = 0;

            static float rendFrameTimeGraph[500] = { 0 };
            static int rendFrameIndex = 0;

            static float updateFrameTimeGraph[500] = { 0 };
            static int updateFrameIndex = 0;

            // Update the rolling graph
            frameTimeGraph[frameIndex] = p_deltaTime * 1000.0f; // Convert seconds to milliseconds
            frameIndex = (frameIndex + 1) % IM_ARRAYSIZE(frameTimeGraph);

            rendFrameTimeGraph[rendFrameIndex] = renderFrameTime; // Convert seconds to milliseconds
            rendFrameIndex = (rendFrameIndex + 1) % IM_ARRAYSIZE(rendFrameTimeGraph);

            updateFrameTimeGraph[updateFrameIndex] = updateFrameTime; // Convert seconds to milliseconds
            updateFrameIndex = (updateFrameIndex + 1) % IM_ARRAYSIZE(updateFrameTimeGraph);

            // Begin ImGui window
            ImGui::Begin("Stats Window");

            // Uptime
            ImGui::Text("Uptime: %.fs", uptime);

            // Display FPS
            ImGui::Text("FPS: %d", fps);

            // Display frame time
            ImGui::Text("Frame Time: %.3f ms", p_deltaTime * 1000.0f);

            // Display render frame time
            ImGui::Text("Render Frame Time: %.3f ms", renderFrameTime);

            // Display update frame time
            ImGui::Text("Update Frame Time: %.3f ms", updateFrameTime);

            ImGui::Text("Draw Calls: %d", drawCalls);

            // Display a graph of frame times
            if (ImPlot::BeginPlot("Frame Time Plot", ImVec2(-1, 150)))
            {
                ImPlot::SetupAxes("Frame Index", "Frame Time (ms)", ImPlotAxisFlags_NoTickLabels, ImPlotAxisFlags_None);
                ImPlot::SetupAxesLimits(0.0, 500.0, 0.0, 30.0);
                ImPlot::PlotLine("Frame Times", frameTimeGraph, IM_ARRAYSIZE(frameTimeGraph));
                ImPlot::PlotLine("Update Frame Times", updateFrameTimeGraph, IM_ARRAYSIZE(updateFrameTimeGraph));
                ImPlot::PlotLine("Render Frame Times", rendFrameTimeGraph, IM_ARRAYSIZE(rendFrameTimeGraph));
                ImPlot::EndPlot();
            };

            ImGui::End();
        }

        void ShowEngineControls(float *timeScale)
        {
            ImGui::Begin("Engine Debug");

            ImGui::SliderFloat("Time scale", timeScale, 0.0f, 2.0f);

            ImGui::End();
        }
    };

} // namespace lum::metrics

#endif // DEBUG_WINDOWS_H