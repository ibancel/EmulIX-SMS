#pragma once

#include "Singleton.h"

#include <vector>
#include <memory>

#include <SFML/Window.hpp>

#include "Breakpoint.h"
#include "Watcher.h"

class Debugger : public Singleton<Debugger>
{
public:
	enum class State { kPaused, kRunning, };
	enum class Action { kNone, kResume, kStep };

	Debugger();

	inline void addBreakpoint(std::unique_ptr<Breakpoint> iNewBreakpoint) {
		_breakpointList.push_back(std::move(iNewBreakpoint));
	}

	inline void addWatcher(std::unique_ptr<Watcher> iNewWatcher) {
		_watcherList.push_back(std::move(iNewWatcher));
	}

	void reset();

	Debugger::State manage(uint_fast64_t iCurrentAddr);
	void captureEvents(const sf::Event& event);

	void pause();
	void resume();
	void step();

	void addNumberCycle(uint_fast64_t iTstatesCycle);

	Debugger::State getState() const;

private:
	std::vector<std::unique_ptr<Breakpoint>> _breakpointList;
	std::vector<std::unique_ptr<Watcher>> _watcherList;
	State _actualState;
	uint_fast64_t _instructionCounter;
	uint_fast64_t _cycleCounter;
	bool _requestStep;
};