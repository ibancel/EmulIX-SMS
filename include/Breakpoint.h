#pragma once

#include <cstdint>

class Breakpoint
{
public:
	enum class Type { kAddress, kNumInstruction, kNumState };

	Breakpoint(const Breakpoint::Type iType, const uint_fast64_t iValue);
	~Breakpoint() = default;

	inline Breakpoint::Type getType() const { return _type; }

	inline uint_fast64_t getValue() const { return _value; }

private:
	Breakpoint::Type _type;
	uint_fast64_t _value;
};