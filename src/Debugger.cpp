#include "Debugger.h"

#include "Breakpoint.h"

Debugger::Debugger() : _instructionCounter{ 0 }, _requestStep{ false }, _actualState{ State::kRunning }
{

}

void Debugger::reset()
{
	_instructionCounter = 0;
	_requestStep = false;
	_actualState = State::kRunning;
}

Debugger::State Debugger::manage(uint_fast64_t iCurrentAddr)
{
	_instructionCounter++;

	for (const std::unique_ptr<Breakpoint>& aBreakpointPtr : _breakpointList) {
		Breakpoint *breakpoint = aBreakpointPtr.get();
		if (!breakpoint) {
			continue;
		}
		if (breakpoint->getType() == Breakpoint::Type::kNumInstruction && breakpoint->getValue() == _instructionCounter) {
			pause();
		} else if (breakpoint->getType() == Breakpoint::Type::kAddress && breakpoint->getValue() == iCurrentAddr) {
			pause();
		}
	}

	if (_actualState == State::kPaused && _requestStep) {
		_requestStep = false;
		return State::kRunning;
	}

	_requestStep = false;
	return _actualState;
}

void Debugger::captureEvents(const sf::Event& event)
{
	if (event.type == sf::Event::KeyReleased)
	{
		if (event.key.code == sf::Keyboard::F5) {
			if (getState() == Debugger::State::kRunning) {
				pause();
			} else {
				resume();
			}
		} else if (event.key.code == sf::Keyboard::F8) {
			step();
		}
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


Debugger::State Debugger::getState() const
{
	return _actualState;
}