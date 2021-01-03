#include "Inputs.h"

#include <QEvent>
#include <QKeyEvent>
#include <QDebug>

#include "Debugger.h"
#include "Graphics.h"


Inputs::Inputs() : 
    QObject(nullptr),
	_debugger{ Debugger::Instance() }, 
    _drawSprite{ true },
    _controller{ },
    _controllerConnected{ true, false },
    _graphics{ nullptr },
    _infoDisplayMode{ 0 },
    _isStopRequested{ false },
    _userKeys{ }
{
    _userKeys[0][ControllerKey::CK_UP]    = Qt::Key_Up;
    _userKeys[0][ControllerKey::CK_DOWN]  = Qt::Key_Down;
    _userKeys[0][ControllerKey::CK_RIGHT] = Qt::Key_Right;
    _userKeys[0][ControllerKey::CK_LEFT]  = Qt::Key_Left;
    _userKeys[0][ControllerKey::CK_FIREA] = Qt::Key_D;
    _userKeys[0][ControllerKey::CK_FIREB] = Qt::Key_F;
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
//    qDebug() << "test key " << idController << " : " << cKey << " = " << _controller[idController][cKey];
    return _controller[idController][cKey];
}

Qt::Key* Inputs::getControllerKeys(JoypadId idController)
{
    return _userKeys[idController];
}

bool Inputs::getDrawSprite()
{
	return _drawSprite;
}

int Inputs::getInfoDisplayMode()
{
	return _infoDisplayMode;
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

void Inputs::setControllerKeys(JoypadId idController, Qt::Key keys[NUMBER_KEYS])
{
    for(int i = 0 ; i < NUMBER_KEYS ; i++) {
        _userKeys[idController][i] = keys[i];
    }
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
            qDebug() << keyEvent->key() << " " << event->type() << " " << keyEvent->isAutoRepeat();
            for(int indexController = 0 ; indexController < NUMBER_JOYSTICK ; indexController++) {
                for (int indexKey = 0; indexKey < 6; indexKey++) {
                    if (_controllerConnected[indexController] && keyPressed == _userKeys[indexController][indexKey]) {
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
//                app->close();
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
