#include "Inputs.h"

#include <QEvent>
#include <QGamepad>
#include <QKeyEvent>
#include <QDebug>

#include "Debugger.h"
#include "Graphics.h"


std::string Inputs::keyToString(Inputs::ControllerKey key)
{
    switch(key) {
        case ControllerKey::CK_DOWN:
            return "Down";
            break;
        case ControllerKey::CK_FIREA:
            return "Button A";
            break;
        case ControllerKey::CK_FIREB:
            return "Button B";
            break;
        case ControllerKey::CK_LEFT:
            return "Left";
            break;
        case ControllerKey::CK_RIGHT:
            return "Right";
            break;
        case ControllerKey::CK_UP:
            return "Up";
            break;
    }
}

Inputs::Inputs() : 
    QObject(nullptr),
	_debugger{ Debugger::Instance() }, 
    _drawSprite{ true },
    _controller{ },
    _controllerConnected{ true, false },
    _controllerType{ ControllerType::kTypeKeyboard, ControllerType::kTypeKeyboard },
    _gamepadListeners{ },
    _graphics{ nullptr },
    _infoDisplayMode{ 0 },
    _isStopRequested{ false },
    _userKeys{ }
{
    _userKeys[0][ControllerKey::CK_UP]    = { ControllerType::kTypeKeyboard, Qt::Key_Up };
    _userKeys[0][ControllerKey::CK_DOWN]  = { ControllerType::kTypeKeyboard, Qt::Key_Down };
    _userKeys[0][ControllerKey::CK_RIGHT] = { ControllerType::kTypeKeyboard, Qt::Key_Right };
    _userKeys[0][ControllerKey::CK_LEFT]  = { ControllerType::kTypeKeyboard, Qt::Key_Left };
    _userKeys[0][ControllerKey::CK_FIREA] = { ControllerType::kTypeKeyboard, Qt::Key_D };
    _userKeys[0][ControllerKey::CK_FIREB] = { ControllerType::kTypeKeyboard, Qt::Key_F };
    refreshAllGamepadListeners();
}

void Inputs::aknowledgeStopRequest()
{
    _isStopRequested = false;
}

void Inputs::connect(JoypadId idController, bool value)
{
    _controllerConnected[idController] = value;
}

bool Inputs::controllerKeyPressed(JoypadId idController, ControllerKey cKey)
{
    return _controller[idController][cKey];
}

bool Inputs::getDrawSprite()
{
	return _drawSprite;
}

int Inputs::getInfoDisplayMode()
{
	return _infoDisplayMode;
}

InputData* Inputs::getUserKeys(JoypadId idController)
{
    return _userKeys[idController];
}

bool Inputs::isConnected(JoypadId idController)
{
    return _controllerConnected[idController];
}

bool Inputs::isStopRequested()
{
	return _isStopRequested;
}

void Inputs::requestStop()
{
    _isStopRequested = true;
}

void Inputs::setUserKeys(JoypadId idController, InputData inputDatas[NUMBER_KEYS])
{
    for(int i = 0 ; i < NUMBER_KEYS ; i++) {
        _userKeys[idController][i] = inputDatas[i];
    }
    refreshAllGamepadListeners();
}

void Inputs::switchDrawSprite()
{
    _drawSprite = !_drawSprite;
}

void Inputs::switchInfoDisplayMode()
{
    _infoDisplayMode++;
    if (_infoDisplayMode > 1) {
        _infoDisplayMode = 0;
    }
}

// Protected:

bool Inputs::eventFilter(QObject* obj, QEvent* event)
{
    if(event->type() == QEvent::KeyPress || event->type() == QEvent::KeyRelease) {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
        if(!keyEvent->isAutoRepeat()) {
            int keyPressed = keyEvent->key();
//            qDebug() << keyEvent->key() << " " << event->type() << " " << keyEvent->isAutoRepeat();
            for(int indexController = 0 ; indexController < NUMBER_JOYPAD ; indexController++) {
                for (int indexKey = 0; indexKey < 6; indexKey++) {
                    if (_controllerConnected[indexController] && _userKeys[indexController][indexKey].type == ControllerType::kTypeKeyboard && keyPressed == std::get<Qt::Key>(_userKeys[indexController][indexKey].data)) {
                        _controller[indexController][indexKey] = (event->type() == QEvent::KeyPress);
                        return true;
                    }
                }
            }
        }
    }
    if(event->type() == QEvent::KeyPress) {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
        if(!keyEvent->isAutoRepeat()) {
            int keyPressed = keyEvent->key();
            if(keyPressed == Qt::Key_Escape) {
                requestStop();
            } else if (keyPressed == Qt::Key_0) {
                switchInfoDisplayMode();
            } else if(keyPressed == Qt::Key_5) {
                switchDrawSprite();
            } else if (keyPressed == Qt::Key_V) {
                _graphics->dumpVram();
            } else if (keyPressed == Qt::Key_R) {
                // TODO(ibancel): singleton
//                Memory::Instance()->dumpRam();
            }

            _debugger->captureEvents(*keyEvent);
            return true;
        }
    }

    return QObject::eventFilter(obj, event);
}

// Private:

void Inputs::addGamepadListeners(int iGamepadId, int indexJoypad)
{
    QGamepad* aGamepad = new QGamepad(iGamepadId, this);
    for(int indexKey = 0 ; indexKey < NUMBER_KEYS ; indexKey++)
    {
        if(_userKeys[indexJoypad][indexKey].type == ControllerType::kTypeJoystick && (std::get<JoystickKeys>(_userKeys[indexJoypad][indexKey].data) == JoystickKeys::kThumbLeftLeft || std::get<JoystickKeys>(_userKeys[indexJoypad][indexKey].data) == JoystickKeys::kThumbLeftRight)) {
            int signAxis = std::get<JoystickKeys>(_userKeys[indexJoypad][indexKey].data) == JoystickKeys::kThumbLeftLeft ? -1 : 1;
            _gamepadListeners.push_back(QObject::connect(aGamepad, &QGamepad::axisLeftXChanged, this, [this, indexJoypad, indexKey, signAxis](double value){
                if(value*signAxis > 0) {
                    _controller[indexJoypad][indexKey] = (abs(value) > Inputs::JOYSTICK_TRESHOLD);
                }
            }));
        }
        else if(_userKeys[indexJoypad][indexKey].type == ControllerType::kTypeJoystick && (std::get<JoystickKeys>(_userKeys[indexJoypad][indexKey].data) == JoystickKeys::kThumbLeftUp || std::get<JoystickKeys>(_userKeys[indexJoypad][indexKey].data) == JoystickKeys::kThumbLeftDown)) {
            int signAxis = std::get<JoystickKeys>(_userKeys[indexJoypad][indexKey].data) == JoystickKeys::kThumbLeftUp ? -1 : 1;
            _gamepadListeners.push_back(QObject::connect(aGamepad, &QGamepad::axisLeftYChanged, this, [this, indexJoypad, indexKey, signAxis](double value){
                if(value*signAxis > 0) {
                    _controller[indexJoypad][indexKey] = (abs(value) > Inputs::JOYSTICK_TRESHOLD);
                }
            }));
        }
        else if(_userKeys[indexJoypad][indexKey].type == ControllerType::kTypeJoystick && std::get<JoystickKeys>(_userKeys[indexJoypad][indexKey].data) == JoystickKeys::kDpadUp) {
            _gamepadListeners.push_back(QObject::connect(aGamepad, &QGamepad::buttonUpChanged, this, [this, indexJoypad, indexKey](bool pressed){
                _controller[indexJoypad][indexKey] = pressed;
            }));
        }
        else if(_userKeys[indexJoypad][indexKey].type == ControllerType::kTypeJoystick && std::get<JoystickKeys>(_userKeys[indexJoypad][indexKey].data) == JoystickKeys::kDpadLeft) {
            _gamepadListeners.push_back(QObject::connect(aGamepad, &QGamepad::buttonLeftChanged, this, [this, indexJoypad, indexKey](bool pressed){
                _controller[indexJoypad][indexKey] = pressed;
            }));
        }
        else if(_userKeys[indexJoypad][indexKey].type == ControllerType::kTypeJoystick && std::get<JoystickKeys>(_userKeys[indexJoypad][indexKey].data) == JoystickKeys::kDpadDown) {
            _gamepadListeners.push_back(QObject::connect(aGamepad, &QGamepad::buttonDownChanged, this, [this, indexJoypad, indexKey](bool pressed){
                _controller[indexJoypad][indexKey] = pressed;
            }));
        }
        else if(_userKeys[indexJoypad][indexKey].type == ControllerType::kTypeJoystick && std::get<JoystickKeys>(_userKeys[indexJoypad][indexKey].data) == JoystickKeys::kDpadRight) {
            _gamepadListeners.push_back(QObject::connect(aGamepad, &QGamepad::buttonRightChanged, this, [this, indexJoypad, indexKey](bool pressed){
                _controller[indexJoypad][indexKey] = pressed;
            }));
        }
        else if(_userKeys[indexJoypad][indexKey].type == ControllerType::kTypeJoystick && std::get<JoystickKeys>(_userKeys[indexJoypad][indexKey].data) == JoystickKeys::kButtonA) {
            _gamepadListeners.push_back(QObject::connect(aGamepad, &QGamepad::buttonAChanged, this, [this, indexJoypad, indexKey](bool pressed){
                _controller[indexJoypad][indexKey] = pressed;
            }));
        }
        else if(_userKeys[indexJoypad][indexKey].type == ControllerType::kTypeJoystick && std::get<JoystickKeys>(_userKeys[indexJoypad][indexKey].data) == JoystickKeys::kButtonB) {
            _gamepadListeners.push_back(QObject::connect(aGamepad, &QGamepad::buttonBChanged, this, [this, indexJoypad, indexKey](bool pressed){
                _controller[indexJoypad][indexKey] = pressed;
            }));
        }
        else if(_userKeys[indexJoypad][indexKey].type == ControllerType::kTypeJoystick && std::get<JoystickKeys>(_userKeys[indexJoypad][indexKey].data) == JoystickKeys::kButtonX) {
            _gamepadListeners.push_back(QObject::connect(aGamepad, &QGamepad::buttonXChanged, this, [this, indexJoypad, indexKey](bool pressed){
                _controller[indexJoypad][indexKey] = pressed;
            }));
        }
        else if(_userKeys[indexJoypad][indexKey].type == ControllerType::kTypeJoystick && std::get<JoystickKeys>(_userKeys[indexJoypad][indexKey].data) == JoystickKeys::kButtonY) {
            _gamepadListeners.push_back(QObject::connect(aGamepad, &QGamepad::buttonYChanged, this, [this, indexJoypad, indexKey](bool pressed){
                _controller[indexJoypad][indexKey] = pressed;
            }));
        }
    }
}

void Inputs::refreshAllGamepadListeners()
{
    for(QMetaObject::Connection aConnection : _gamepadListeners) {
        disconnect(aConnection);
    }
    _gamepadListeners.clear();

    std::set<int> gamepadSet[2];
    for(int indexJoypad = 0 ; indexJoypad < NUMBER_JOYPAD ; indexJoypad++) {
        for(int indexKey = 0 ; indexKey < NUMBER_KEYS ; indexKey++) {
            if(_controllerConnected[indexJoypad] && _userKeys[indexJoypad][indexKey].type == ControllerType::kTypeJoystick && gamepadSet[indexJoypad].find(_userKeys[indexJoypad][indexKey].joystickId) == gamepadSet[indexJoypad].end()) {
                gamepadSet[indexJoypad].insert(_userKeys[indexJoypad][indexKey].joystickId);
                addGamepadListeners(_userKeys[indexJoypad][indexKey].joystickId, indexJoypad);
            }
        }
    }
}
