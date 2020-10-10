#include "Debugger.h"

#include <QKeyEvent>

#include "Breakpoint.h"
#include "Log.h"
#include "Memory.h"

Debugger::Debugger() : _instructionCounter{ 0 }, _cycleCounter{ 5 }, _requestStep{ false }, _actualState{ State::kRunning }
{

}

void Debugger::reset()
{
	_instructionCounter = 0;
	_cycleCounter = 0;
	_requestStep = false;
	_actualState = State::kRunning;
}

Debugger::State Debugger::manage(uint_fast64_t iCurrentAddr)
{
	_instructionCounter++;

	if (_actualState == State::kRunning) {
		for (std::vector<std::unique_ptr<Breakpoint>>::iterator it = _breakpointList.begin(); it != _breakpointList.end();) {
			Breakpoint* breakpoint = it->get();
			if (!breakpoint) {
				++it;
				continue;
			}

			bool toErase = false;
			if (breakpoint->getType() == Breakpoint::Type::kNumInstruction && breakpoint->getValue() == _instructionCounter) {
				pause();
			} else if (breakpoint->getType() == Breakpoint::Type::kAddress && breakpoint->getValue() == iCurrentAddr) {
				pause();
			} else if (breakpoint->getType() == Breakpoint::Type::kNumState && breakpoint->getValue() <= _cycleCounter) {
				pause();
				toErase = true;
			}

			if (toErase) {
				_breakpointList.erase(it);
			} else {
				++it;
			}
		}
	}

	if (_actualState == State::kRunning) {
		for (std::unique_ptr<Watcher>& watcher : _watcherList) {
			if (!watcher) {
				continue;
			}

			uint8_t memoryValue = Memory::Instance()->read(watcher->getAddress());
			if (memoryValue != watcher->getCurrentValue()) {
				pause();
				SLOG(ldebug << "Watcher " << watcher->getAddress() << " (" << _cycleCounter << ")");
				watcher->setCurrentValue(memoryValue);
			}
		}
	}



	if (_actualState == State::kPaused && _requestStep) {
		_requestStep = false;
		return State::kRunning;
	}

	_requestStep = false;
	return _actualState;
}

void Debugger::captureEvents(const QKeyEvent& iKeyEvent)
{
    int keyPressed = iKeyEvent.key();
    if (keyPressed == Qt::Key_F5) {
        if (getState() == Debugger::State::kRunning) {
            pause();
        } else {
            resume();
        }
    } else if (keyPressed == Qt::Key_F8) {
        step();
    }

}

void Debugger::pause()
{
	_actualState = Debugger::State::kPaused;
}

void Debugger::resume()
{
	_actualState = Debugger::State::kRunning;
}

void Debugger::step()
{
	_requestStep = true;
}

void Debugger::addNumberCycle(uint_fast64_t iTstatesCycle)
{
	_cycleCounter += iTstatesCycle;
}

Debugger::State Debugger::getState() const
{
	return _actualState;
}
