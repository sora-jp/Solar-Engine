#include "pch.h"
#include "Subsystem.h"

bool Subsystem::operator<(const Subsystem& b) const
{
	return GetOrder() < b.GetOrder();
}