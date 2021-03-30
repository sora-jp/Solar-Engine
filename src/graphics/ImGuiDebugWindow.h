#pragma once
#include "imgui.h"
#include "implot/implot.h"
#include "diligent/DiligentInit.h"
#include "core/Profiler.h"
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
	double engineTime;
	double renderTime;
	double vsyncTime;
};

struct ScrollingBuffer {
	uint64_t MaxSize;
	int Offset;
	std::vector<DebugProfilerData> Data;

	ScrollingBuffer(uint64_t max_size = 2000) {
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

	[[nodiscard]] double MaxTime(double timeLimit) const
	{
		double max = 0;
		for (auto i = 1ull; i <= Data.size(); i++)
		{
			auto idx = static_cast<int>((Data.size() < MaxSize ? Data.size() : Offset) - i);
			while (idx < 0) idx += MaxSize;

			auto& d = Data[idx];
			if (d.totalFrameTime > max) max = d.totalFrameTime;
			if (d.timestamp < timeLimit) break;
		}

		//SOLAR_CORE_TRACE("Max at {}", std::distance(Data.begin(), it));
		return max;
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

inline void DrawDebugWindow(Shared<DiligentContext> ctx, ImFont* font, ImFont* smallFont)
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
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoBringToFrontOnFocus;

	const auto pad = 10.0f;
	const ImGuiViewport* viewport = ImGui::GetMainViewport();

	ImVec2 windowPos;

	windowPos.x = (viewport->WorkPos.x + pad);
	windowPos.y = (viewport->WorkPos.y + pad);

	ImGui::SetNextWindowPos(windowPos, ImGuiCond_Always, ImVec2(0, 0));
	ImGui::SetNextWindowViewport(viewport->ID);
	ImGui::SetNextWindowBgAlpha(0.35f);

	ImGui::PushFont(font);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(5, 5));
	
	ImGui::Begin("Pipeline statistics", nullptr, windowFlags);
	ImGui::Text("Verts: %llu, Tris: %llu", stats.InputVertices, stats.InputPrimitives);
	ImGui::Text(u8"GPU Time: %.1fµs", duration * 1e6f);

	if (Profiler::HasRootNodes()) {
		static float t = 0;

		DebugProfilerData data {};
		data.timestamp = t;
		data.totalFrameTime = Profiler::GetRootNode()->TotalMs;
		data.engineTime = Profiler::GetTimeForCategory("Engine");
		data.renderTime = Profiler::GetTimeForCategory("Rendering");
		data.vsyncTime  = Profiler::GetTimeForCategory("VSync");

		if (s_frameBudgetUsage < 0) s_frameBudgetUsage = (1.0f - data.vsyncTime / data.totalFrameTime) * 100;
		else s_frameBudgetUsage = ((1.0f - data.vsyncTime / data.totalFrameTime) * 100) * SmoothFactor + s_frameBudgetUsage * (1.0f - SmoothFactor);
		
		ImGui::Text("Total time: %.1fms", s_dataQueue.MaxTime(t - 4));
		ImGui::Text("Frame usage: %.1f%%", s_frameBudgetUsage);
		data.totalFrameTime -= data.vsyncTime;

		//if (ImGui::BeginTable("Profiler", 2, ImGuiTableFlags_SizingStretchProp, ImVec2(ImGui::GetContentRegionAvailWidth(), 0))) {
		//	DrawProfilerBar(data.engineTime, data.totalFrameTime);
		//	DrawProfilerBar(data.renderTime, data.totalFrameTime);
		//	DrawProfilerBar(data.vsyncTime,  data.totalFrameTime);

		//	ImGui::EndTable();
		//}

		data.renderTime += data.engineTime;
		data.vsyncTime += data.renderTime;
		s_dataQueue.Add(data);
		
		static const auto FLAGS = ImPlotAxisFlags_NoTickLabels | ImPlotAxisFlags_NoDecorations;

		ImPlot::SetNextPlotLimitsX(t - 4.f, t, ImGuiCond_Always);
		ImPlot::SetNextPlotLimitsY(0, s_dataQueue.MaxTime(t - 4), ImGuiCond_Always);
		
		ImPlot::PushStyleVar(ImPlotStyleVar_PlotBorderSize, 0.f);
		ImPlot::PushStyleVar(ImPlotStyleVar_LegendPadding, ImVec2(0, 0));
		ImPlot::PushStyleVar(ImPlotStyleVar_PlotPadding, ImVec2(0, 0));

		ImPlot::PushStyleColor(ImPlotCol_LegendBg, 0);
		ImPlot::PushStyleColor(ImPlotCol_LegendBorder, 0);

		ImGui::PushFont(smallFont);
		
		if (ImPlot::BeginPlot("##Scrolling", nullptr, nullptr, ImVec2(ImGui::GetWindowContentRegionWidth(), 100), ImPlotFlags_NoMousePos | ImPlotFlags_NoMenus, FLAGS, FLAGS)) {
			ImPlot::SetLegendLocation(ImPlotLocation_South, ImPlotOrientation_Horizontal, true);

			const auto stride = sizeof(DebugProfilerData);
			const auto offset = s_dataQueue.Offset;
			const auto size = s_dataQueue.Data.size();

			const auto& data1 = s_dataQueue.Data[0];

			//ImPlot::SetNextFillStyle(ImVec4(0.372, 0.827, 0.443, -1));
			//ImPlot::PlotShaded("Vsync",  &data1.timestamp, &data1.vsyncTime , size, -INFINITY, offset, stride);
			
			//ImPlot::SetNextFillStyle(ImVec4(0.827, 0.619, 0.372, -1));
			ImPlot::PlotShaded("Render", &data1.timestamp, &data1.renderTime, size, -INFINITY, offset, stride);
			
			//ImPlot::SetNextFillStyle(ImVec4(0.372, 0.717, 0.827, -1));
			ImPlot::PlotShaded("PreRun", &data1.timestamp, &data1.engineTime, size, -INFINITY, offset, stride);
			
			ImPlot::EndPlot();
		}
		
		ImGui::PopFont();

		ImPlot::PopStyleColor(2);
		ImPlot::PopStyleVar(3);
		
		t += 1.f / 75;//ImGui::GetIO().DeltaTime;
	}
	
	ImGui::End();
	ImGui::PopStyleVar();
	ImGui::PopFont();
	
	ImPlot::ShowDemoWindow();
}
