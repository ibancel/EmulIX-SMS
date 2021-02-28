#pragma once

#include "Singleton.h"

#include <memory>
#include <vector>

#include <QKeyEvent>

#include "Breakpoint.h"
#include "Memory.h"
#include "Watcher.h"

class Memory;

class Debugger : public Singleton<Debugger>
{
public:
	enum class State {
		kPaused,
		kRunning,
	};
	enum class Action { kNone, kResume, kStep };

	Debugger();

	inline void addBreakpoint(std::unique_ptr<Breakpoint> iNewBreakpoint)
	{
		_breakpointList.push_back(std::move(iNewBreakpoint));
	}

	inline void addWatcher(std::unique_ptr<Watcher> iNewWatcher) { _watcherList.push_back(std::move(iNewWatcher)); }

	void reset();

	Debugger::State manage(uint_fast64_t iCurrentAddr);
	void captureEvents(const QKeyEvent& iKeyEvent);

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
	Memory* _memory;
	bool _requestStep;
};
