#pragma once
#include <string>
#include "imgui.h"
#include "diligent/DiligentInit.h"
#include "core/Profiler.h"

//inline std::string DurationText(double timeMs)
//{
//	std::string str(16, '\0');
//	_sprintf_p(str.data(), str.capacity(), "%.1fms", timeMs);
//	return str;
//}

inline void DrawProfilerBar(const double duration, const double totalTime)
{
	ImGui::TableNextColumn();
	ImGui::Text("%.1fms", duration);
	
	ImGui::TableNextColumn();
	ImGui::ProgressBar(duration / totalTime);
}

const static double SmoothFactor = .25f;
static double s_frameBudgetUsage = -1;

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
		const auto totalFrameTime = Profiler::GetRootNode()->TotalMs;
		const auto prerunTime = Profiler::GetNodeFromPath("PreRun")->TotalMs;
		const auto renderTime = Profiler::GetNodeFromPath("PostRun/Render")->TotalMs;
		const auto vsyncTime  = Profiler::GetNodeFromPath("PostRun/VSync")->TotalMs;

		if (s_frameBudgetUsage < 0) s_frameBudgetUsage = (1.0f - vsyncTime / totalFrameTime) * 100;
		else s_frameBudgetUsage = ((1.0f - vsyncTime / totalFrameTime) * 100) * SmoothFactor + s_frameBudgetUsage * (1.0f - SmoothFactor);
		
		ImGui::Text("Frame usage: %.1f%%", s_frameBudgetUsage);

		if (ImGui::BeginTable("Profiler", 2, ImGuiTableFlags_SizingStretchProp, ImVec2(ImGui::GetContentRegionAvailWidth(), 0))) {
			DrawProfilerBar(prerunTime, totalFrameTime);
			DrawProfilerBar(renderTime, totalFrameTime);
			DrawProfilerBar(vsyncTime, totalFrameTime);

			ImGui::EndTable();
		}
	}
	
	ImGui::End();
	ImGui::PopFont();
}
