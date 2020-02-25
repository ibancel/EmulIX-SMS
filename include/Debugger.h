#pragma once

#include "Singleton.h"

#include <vector>
#include <memory>

#include <SFML/Window.hpp>

#include "Breakpoint.h"

class Debugger : public Singleton<Debugger>
{
public:
	enum class State { kPaused, kRunning, };
	enum class Action { kNone, kResume, kStep };

	Debugger();

	inline void addBreakpoint(std::unique_ptr<Breakpoint> iNewBreakpoint) {
		_breakpointList.push_back(std::move(iNewBreakpoint));
	}

	void reset();

	Debugger::State manage(uint_fast64_t iCurrentAddr);
	void captureEvents(const sf::Event& event);

	void pause();
	void resume();
	void step();

	Debugger::State getState() const;

private:
	std::vector<std::unique_ptr<Breakpoint>> _breakpointList;
	State _actualState;
	uint_fast64_t _instructionCounter;
	bool _requestStep;
};