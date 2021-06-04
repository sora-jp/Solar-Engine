#pragma once
#include "Common.h"
#include <chrono>
#include <utility>
#include <map>

#ifndef SOLAR_PROFILE
#define SOLAR_PROFILE 0
#endif

struct ProfilerNode
{
	friend class Profiler;
	friend class Engine;
	friend Shared<ProfilerNode> CreateChild(const Shared<ProfilerNode>&, std::string, std::string);
	
	Weak<ProfilerNode> Parent;
	const std::string Name;
	const std::string Category;
	double TimeMs, TotalMs;
	std::vector<Shared<ProfilerNode>> Children;

	[[nodiscard]] double SelfPercentOfTotal() const { return TimeMs / TotalMs; }
	[[nodiscard]] double TotalWithoutSelf() const { return TotalMs - TimeMs; }
	
private:
	ProfilerNode(const Shared<ProfilerNode>& parent, std::string name, std::string category) : Parent(parent), Name(std::move(name)), Category(std::move(category)), TimeMs(0), TotalMs(0) {}
};

class SOLAR_API Profiler
{
	static Shared<ProfilerNode> _root;
	static Shared<ProfilerNode> _lastRoot;
	static Shared<ProfilerNode> _curNode;

	static std::map<std::string, double> _categoryTimes;
	static std::map<std::string, double> _lastCategoryTimes;
	static std::chrono::high_resolution_clock::time_point _curNodeStart;

public:
	Profiler() = delete;
	~Profiler() = delete;

#if SOLAR_PROFILE
	static void Begin(std::string name, std::string category = "Unknown");
	static void End();
	
	static void BeginRoot();
	static void EndRoot();

	static bool HasRootNodes();
	static Shared<ProfilerNode> GetNodeFromPath(const std::string& path);
	static Shared<ProfilerNode> GetRootNode();
	static std::vector<Shared<ProfilerNode>>& GetRootNodes();
	static double GetTimeForCategory(const std::string& category);
#else
	static void Begin(std::string name, std::string category = "Unknown") {}
	static void End() {}
	
	static void BeginRoot() {}
	static void EndRoot() {}

	static bool HasRootNodes() { return false; }
	static Shared<ProfilerNode> GetNodeFromPath(const std::string& path) { return nullptr; }
	static Shared<ProfilerNode> GetRootNode() { return nullptr; }
	static std::vector<Shared<ProfilerNode>>& GetRootNodes() { return std::vector<Shared<ProfilerNode>>(); }
	static double GetTimeForCategory(const std::string& category) { return 0; }
#endif
};