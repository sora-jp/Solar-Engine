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

#define SUBSYSTEM_ORDER(order) int GetOrder() const override { return order; }
#define SUBSYSTEM_NAME(name) const char* GetName() const override { return name; }

//INSTANTIATE_FACTORY_DEF(Subsystem)
//#define REGISTER_SUBSYSTEM(sys) REGISTER(Subsystem, sys)
//#define GET_SUBSYSTEMS() GET(Subsystem)