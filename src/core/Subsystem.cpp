#include "pch.h"
#include "Subsystem.h"

//INSTANTIATE_FACTORY(Subsystem);

bool Subsystem::operator<(const Subsystem& b) const
{
	return GetOrder() < b.GetOrder();
}