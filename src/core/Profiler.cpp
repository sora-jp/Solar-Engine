#include "pch.h"
#include "Profiler.h"

SOLAR_API Shared<ProfilerNode> Profiler::_root;
SOLAR_API Shared<ProfilerNode> Profiler::_lastRoot;
SOLAR_API Shared<ProfilerNode> Profiler::_curNode;
SOLAR_API std::map<std::string, double> Profiler::_categoryTimes;
SOLAR_API std::chrono::high_resolution_clock::time_point Profiler::_curNodeStart;

Shared<ProfilerNode> CreateChild(const Shared<ProfilerNode>& parent, std::string name, std::string category)
{
	auto node = Shared<ProfilerNode>(new ProfilerNode(parent, std::move(name), std::move(category)));
	if (parent) parent->Children.push_back(node);

	return node;
}

void Profiler::Begin(std::string name, std::string category)
{
	_curNode = CreateChild(_curNode, std::move(name), std::move(category));
	_curNodeStart = std::chrono::high_resolution_clock::now();
}

void Profiler::End()
{
	SOLAR_ASSERT(_curNode != nullptr);
	
	const auto diff = _curNodeStart - std::chrono::high_resolution_clock::now();
	const auto msDiff = std::chrono::duration<double, std::milli>(diff).count();

	_curNode->TotalMs += (_curNode->TimeMs = msDiff);

	auto pos = _categoryTimes.try_emplace(_categoryTimes.cbegin(), _curNode->Category, 0);
	pos->second += msDiff;

	const auto hasParent = !_curNode->Parent.expired();
	
	if (hasParent) {
		auto parent = _curNode->Parent.lock();
		parent->TotalMs += _curNode->TotalMs;
		_curNode = parent;
	}
	else _curNode = nullptr;
}

void Profiler::BeginRoot()
{
	SOLAR_ASSERT(_curNode == nullptr);
	_categoryTimes.clear();
	
	Begin("Root", "Root");
	_root = _curNode;
}

void Profiler::EndRoot()
{
	SOLAR_ASSERT(_curNode == nullptr);
	_lastRoot = _root;
}

bool Profiler::HasRootNodes()
{
	return (_curNode ? _lastRoot : _root) != nullptr;
}

std::vector<Shared<ProfilerNode>>& Profiler::GetRootNodes()
{
	return (_curNode ? _lastRoot : _root)->Children;
}

double Profiler::GetTimeForCategory(const std::string& category)
{
	return _categoryTimes[category];
}
