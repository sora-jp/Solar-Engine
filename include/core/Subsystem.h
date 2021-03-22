#pragma once
#include "Common.h"

class SUBSYSTEM_API Subsystem
{
public:
	[[nodiscard]] virtual int GetOrder() const { return 0; }
	[[nodiscard]] virtual const char* GetName() const { return "Unknown"; }
	
	virtual ~Subsystem() = default;
	virtual void Init() = 0;
	virtual void PreRun() = 0;
	virtual void PostRun() = 0;
	virtual void Shutdown() = 0;
	virtual bool RequestedShutdown() { return false; }
	
	bool operator<(const Subsystem& b) const;
};

//typedef std::shared_ptr<Subsystem> SubsystemPtr;
// , std::enable_if_t<std::is_base_of_v<Subsystem, T>, bool> = true
template <class T> using SubsystemPtr = Shared<T>;
typedef Subsystem* (__cdecl *CreateSubsystemFunc)();

#define SUBSYSTEM_ORDER(order) int GetOrder() const override { return order; }
#define SUBSYSTEM_NAME(name) const char* GetName() const override { return name; }

#define EXPORT_SUBSYSTEM(sub) extern "C" SUBSYSTEM_API sub* __cdecl CreateSubsystem() { return new sub(); }