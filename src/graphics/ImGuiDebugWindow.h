#pragma once
#include <string>
#include "imgui.h"
#include "diligent/DiligentInit.h"
#include "core/Profiler.h"
#include <algorithm>
#include <vector>
#include <cmath>

//inline std::string DurationText(double timeMs)
//{
//	std::string str(16, '\0');
//	_sprintf_p(str.data(), str.capacity(), "%.1fms", timeMs);
//	return str;
//}

struct DebugProfilerData
{
	double timestamp;
	double totalFrameTime;
	double prerunTime;
	double renderTime;
	double vsyncTime;
};

struct ScrollingBuffer {
	int MaxSize;
	int Offset;
	std::vector<DebugProfilerData> Data;

	ScrollingBuffer(int max_size = 2000) {
		MaxSize = max_size;
		Offset = 0;
		Data.reserve(MaxSize);
	}

	void Add(const DebugProfilerData& data) {
		if (Data.size() < MaxSize)
			Data.push_back(data);
		else {
			Data[Offset] = data;
			Offset = (Offset + 1) % MaxSize;
		}
	}

	double MaxTime() const
	{
		auto it = std::max_element(
			Data.begin() + Offset, 
			Data.end(), 
			[](DebugProfilerData a, DebugProfilerData b) -> bool { return a.totalFrameTime < b.totalFrameTime; });

		//SOLAR_CORE_TRACE("Max at {}", std::distance(Data.begin(), it));
		return it->totalFrameTime;
	}

	void Erase() {
		if (!Data.empty()) {
			Data.resize(0);
			Offset = 0;
		}
	}
};

inline void DrawProfilerBar(const double duration, const double totalTime)
{
	ImGui::TableNextColumn();
	ImGui::Text("%.1fms", duration);
	
	ImGui::TableNextColumn();
	ImGui::ProgressBar(duration / totalTime);
}

const static double SmoothFactor = .25f;
static double s_frameBudgetUsage = -1;

static ScrollingBuffer s_dataQueue;
static unsigned int s_idx;
static unsigned int s_length;

inline void DrawDebugWindow(Shared<DiligentContext> ctx, ImFont* font)
{
	const auto& stats = ctx->GetPipelineStats();
	const auto& duration = ctx->GetLastDuration();

	auto windowFlags =
		ImGuiWindowFlags_NoDecoration |
		ImGuiWindowFlags_NoDocking |
		ImGuiWindowFlags_AlwaysAutoResize |
		ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_NoFocusOnAppearing |
		ImGuiWindowFlags_NoNav |
		ImGuiWindowFlags_NoInputs |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoBringToFrontOnFocus;

	const auto pad = 10.0f;
	const ImGuiViewport* viewport = ImGui::GetMainViewport();

	ImVec2 windowPos, windowPivot;

	windowPos.x = (viewport->WorkPos.x + pad);
	windowPos.y = (viewport->WorkPos.y + pad);

	windowPivot.x = 0.0f;
	windowPivot.y = 0.0f;

	ImGui::SetNextWindowPos(windowPos, ImGuiCond_Always, windowPivot);
	ImGui::SetNextWindowViewport(viewport->ID);
	ImGui::SetNextWindowBgAlpha(0.35f);
	//windowFlags |= ImGuiWindowFlags_NoMove;

	ImGui::PushFont(font);
	ImGui::Begin("Pipeline statistics", nullptr, windowFlags);
	ImGui::Text("Verts: %llu, Tris: %llu", stats.InputVertices, stats.InputPrimitives);
	ImGui::Text(u8"GPU Time: %.1fµs", duration * 1e6f);

	if (Profiler::HasRootNodes()) {
		static float t = 0;
		t += ImGui::GetIO().DeltaTime;

		DebugProfilerData data;
		data.timestamp = t;
		data.totalFrameTime = Profiler::GetRootNode()->TotalMs;
		data.prerunTime = Profiler::GetNodeFromPath("PreRun")->TotalMs;
		data.renderTime = Profiler::GetNodeFromPath("PostRun/Render")->TotalMs;
		data.vsyncTime  = Profiler::GetNodeFromPath("PostRun/VSync")->TotalMs;

		if (s_frameBudgetUsage < 0) s_frameBudgetUsage = (1.0f - data.vsyncTime / data.totalFrameTime) * 100;
		else s_frameBudgetUsage = ((1.0f - data.vsyncTime / data.totalFrameTime) * 100) * SmoothFactor + s_frameBudgetUsage * (1.0f - SmoothFactor);
		
		ImGui::Text("Total time: %.1fms", s_dataQueue.MaxTime());
		ImGui::Text("Frame usage: %.1f%%", s_frameBudgetUsage);

		if (ImGui::BeginTable("Profiler", 2, ImGuiTableFlags_SizingStretchProp, ImVec2(ImGui::GetContentRegionAvailWidth(), 0))) {
			DrawProfilerBar(data.prerunTime, data.totalFrameTime);
			DrawProfilerBar(data.renderTime, data.totalFrameTime);
			DrawProfilerBar(data.vsyncTime,  data.totalFrameTime);

			ImGui::EndTable();
		}

		data.renderTime += data.prerunTime;
		data.vsyncTime += data.renderTime;
		s_dataQueue.Add(data);
		
		static ImPlotAxisFlags flags = ImPlotAxisFlags_NoTickLabels;
		ImPlot::SetNextPlotLimitsX(t - 4, t, ImGuiCond_Always);
		ImPlot::SetNextPlotLimitsY(0, s_dataQueue.MaxTime(), ImGuiCond_Always);
		
		if (ImPlot::BeginPlot("##Scrolling", NULL, NULL, ImVec2(-1, 150), ImPlotFlags_NoMousePos | ImPlotFlags_NoMenus, flags, flags)) {
			ImPlot::SetNextFillStyle(IMPLOT_AUTO_COL, 0.5f);

			const auto stride = sizeof(DebugProfilerData);
			const auto offset = s_dataQueue.Offset;
			const auto size = s_dataQueue.Data.size();

			const auto& data1 = s_dataQueue.Data[0];

			ImPlot::SetNextFillStyle(ImVec4(0.372, 0.827, 0.443, -1));
			ImPlot::PlotShaded("Vsync",  &data1.timestamp, &data1.vsyncTime , size, -INFINITY, offset, stride);
			
			ImPlot::SetNextFillStyle(ImVec4(0.827, 0.619, 0.372, -1));
			ImPlot::PlotShaded("Render", &data1.timestamp, &data1.renderTime, size, -INFINITY, offset, stride);
			
			ImPlot::SetNextFillStyle(ImVec4(0.372, 0.717, 0.827, -1));
			ImPlot::PlotShaded("PreRun", &data1.timestamp, &data1.prerunTime, size, -INFINITY, offset, stride);
			
			ImPlot::EndPlot();
		}
	}
	
	ImGui::End();
	ImGui::PopFont();
}
