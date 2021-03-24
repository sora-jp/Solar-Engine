#include "pch.h"
#include "Profiler.h"

SOLAR_API ProfilerNode* Profiler::_root;
SOLAR_API ProfilerNode* Profiler::_curNode;
SOLAR_API std::chrono::high_resolution_clock::time_point Profiler::_curNodeStart;

ProfilerNode* ProfilerNode::CreateChild(std::string name)
{
	const ProfilerNode node(this, std::move(name));

	Children.push_back(node);
	return &Children.back();
}

void Profiler::Begin(std::string name)
{
	_curNode = _curNode->CreateChild(std::move(name));
	_curNodeStart = std::chrono::high_resolution_clock::now();
}

void Profiler::End()
{
	const auto diff = _curNodeStart - std::chrono::high_resolution_clock::now();
	const auto msDiff = std::chrono::duration<double, std::milli>(diff).count();

	_curNode->TimeMs = msDiff;
	_curNode = _curNode->Parent;
}

const ProfilerNode& Profiler::GetRootNode()
{
	return *_root;
}
