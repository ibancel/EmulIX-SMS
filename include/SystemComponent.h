#pragma once

#include "types.h"

class System;

class SystemComponent
{
public:
	SystemComponent(System& iSystem) : _system { iSystem } { }
	virtual ~SystemComponent() = default;

	virtual u8 read(u16 address) = 0;
	virtual void write(u16 address, u8 data) = 0;

protected:
	System& _system;
};
