#pragma once
#include "Common.h"
#include <chrono>
#include <utility>

struct ProfilerNode
{
	friend class Profiler;
	
	ProfilerNode* Parent;
	const std::string Name;
	double TimeMs;
	std::vector<ProfilerNode> Children;

private:
	ProfilerNode(ProfilerNode* const parent, std::string name) : Parent(parent), Name(std::move(name)), TimeMs(-1) {}
	ProfilerNode* CreateChild(std::string name);
};

class SOLAR_API Profiler
{
	static ProfilerNode* _root;
	static ProfilerNode* _curNode;
	static std::chrono::high_resolution_clock::time_point _curNodeStart;

public:
	Profiler() = delete;
	~Profiler() = delete;

	static void Begin(std::string name);
	static void End();

	static const ProfilerNode& GetRootNode();
};