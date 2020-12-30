#include "Inputs.h"

#include <QEvent>
#include <QKeyEvent>
#include <QDebug>

#include "Debugger.h"
#include "Graphics.h"


Inputs::Inputs() : 
    QObject(nullptr),
	_debugger{ Debugger::Instance() }, 
    // TODO(ibancel): singleton
    _graphics{ nullptr },
	_controller{ false },
	_infoDisplayMode{ 0 },
	_drawSprite{ true },
	_isStopRequested{ false }
{
    _userKeys[ControllerKey::CK_UP]    = Qt::Key_Up;
    _userKeys[ControllerKey::CK_DOWN]  = Qt::Key_Down;
    _userKeys[ControllerKey::CK_RIGHT] = Qt::Key_Right;
    _userKeys[ControllerKey::CK_LEFT]  = Qt::Key_Left;
    _userKeys[ControllerKey::CK_FIREA] = Qt::Key_D;
    _userKeys[ControllerKey::CK_FIREB] = Qt::Key_F;
}

void Inputs::aknowledgeStopRequest()
{
    _isStopRequested = false;
}

bool Inputs::controllerKeyPressed(JoypadId idController, ControllerKey cKey)
{
//    qDebug() << "test key " << idController << " : " << cKey << " = " << _controller[idController][cKey];
    return _controller[idController][cKey];
}

void Inputs::requestStop()
{
    _isStopRequested = true;
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

bool Inputs::getDrawSprite()
{
	return _drawSprite;
}

int Inputs::getInfoDisplayMode()
{
	return _infoDisplayMode;
}

bool Inputs::isStopRequested()
{
	return _isStopRequested;
}

// Protected:

bool Inputs::eventFilter(QObject* obj, QEvent* event)
{
    if(event->type() == QEvent::KeyPress || event->type() == QEvent::KeyRelease) {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
        if(!keyEvent->isAutoRepeat()) {
            int keyPressed = keyEvent->key();
//            qDebug() << keyEvent->key() << " " << event->type() << " " << keyEvent->isAutoRepeat();
            for (int i = 0; i < 6; i++) {
                if (keyPressed == _userKeys[i]) {
                    _controller[JoypadId::kJoypad1][i] = (event->type() == QEvent::KeyPress);
                    return true;
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
