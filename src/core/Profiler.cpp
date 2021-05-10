#include "pch.h"
#include "Profiler.h"

SOLAR_API Shared<ProfilerNode> Profiler::_root;
SOLAR_API Shared<ProfilerNode> Profiler::_lastRoot;
SOLAR_API Shared<ProfilerNode> Profiler::_curNode;
SOLAR_API std::map<std::string, double> Profiler::_lastCategoryTimes;
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
	const auto diff = std::chrono::high_resolution_clock::now() - _curNodeStart;
	const auto msDiff = std::chrono::duration<double, std::milli>(diff).count();

	if (_curNode) _curNode->TotalMs += (_curNode->TimeMs = msDiff);
	
	_curNode = CreateChild(_curNode, std::move(name), std::move(category));
	_curNodeStart = std::chrono::high_resolution_clock::now();
}

void Profiler::End()
{
	SOLAR_ASSERT(_curNode != nullptr);
	
	const auto diff = std::chrono::high_resolution_clock::now() - _curNodeStart;
	const auto msDiff = std::chrono::duration<double, std::milli>(diff).count();

	_curNode->TotalMs += (_curNode->TimeMs += msDiff);

	_categoryTimes[_curNode->Category] += _curNode->TimeMs;

	const auto hasParent = !_curNode->Parent.expired();
	
	if (hasParent) {
		auto parent = _curNode->Parent.lock();
		parent->TotalMs += _curNode->TotalMs;
		_curNode = parent;
	}
	else _curNode = nullptr;

	_curNodeStart = std::chrono::high_resolution_clock::now();
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
	SOLAR_ASSERT(_curNode == _root);
	_lastRoot = _root;
	_curNode = nullptr;

	_lastCategoryTimes = _categoryTimes;
}

bool Profiler::HasRootNodes()
{
	return GetRootNode() != nullptr;
}

Shared<ProfilerNode> Profiler::GetNodeFromPath(const std::string& path)
{
	size_t last = 0;
	size_t next = 0;
	auto node = GetRootNode();
	
	while ((next = path.find('/', last)) != std::string::npos && node != nullptr)
	{
		const auto nextName = path.substr(last, next - last);
		node = *std::find_if(node->Children.begin(), node->Children.end(), [&](const Shared<ProfilerNode>& n) { return n->Name == nextName; });

		if (node->Children.empty()) break;
		last = next + 1;
	}

	if (node != nullptr && !node->Children.empty())
	{
		const auto nextName = path.substr(last);
		node = *std::find_if(node->Children.begin(), node->Children.end(), [&](const Shared<ProfilerNode>& n) { return n->Name == nextName; });
	}

	return node == nullptr ? Shared<ProfilerNode>(new ProfilerNode(nullptr, "Null", "Null")) : node;
}

std::vector<Shared<ProfilerNode>>& Profiler::GetRootNodes()
{
	return GetRootNode()->Children;
}

Shared<ProfilerNode> Profiler::GetRootNode()
{
	return _curNode ? _lastRoot : _root;
}

double Profiler::GetTimeForCategory(const std::string& category)
{
	return _curNode ? _lastCategoryTimes[category] : _categoryTimes[category];
}
