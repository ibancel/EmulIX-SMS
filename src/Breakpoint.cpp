#include "Breakpoint.h"

Breakpoint::Breakpoint(const Breakpoint::Type iType, const uint_fast64_t iValue)
	: _type { iType }, _value { iValue } { }