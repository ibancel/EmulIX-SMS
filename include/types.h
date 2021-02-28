#pragma once

#include <cstdint>

using u8 = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;

using s8 = std::int8_t;
using s16 = std::int16_t;
using s32 = std::int32_t;

template <typename TemplateClass> class PtrRef
{
public:
	PtrRef(TemplateClass* const& iPtrRef) : _ptrRef { iPtrRef } { }

	PtrRef(const PtrRef& iCopy) { _ptrRef = iCopy._ptrRef; }

	TemplateClass* const& ptr() { return _ptrRef; }

	PtrRef operator=(const PtrRef& iCopy)
	{
		_ptrRef = iCopy._ptrRef;
		return *this;
	}

private:
	TemplateClass* const& _ptrRef;
};
