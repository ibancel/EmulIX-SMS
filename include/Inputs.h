#pragma once

#include <QObject>

#include "definitions.h"
#include "Debugger.h"
#include "Graphics.h"
#include "Singleton.h"

class Debugger;
class Graphics;

class Inputs : public QObject, public Singleton<Inputs>
{
    Q_OBJECT
public:
    
    enum JoypadId : size_t { kJoypad1 = 0, kJoypad2 = 1 };
    enum ControllerKey { CK_UP = 0, CK_DOWN = 1, CK_RIGHT = 2, CK_LEFT = 3, CK_FIREA = 4, CK_FIREB = 5 };

    Inputs();
    virtual ~Inputs() = default;

    void aknowledgeStopRequest();

    // idController is 0 for Joypad 1 and 1 for Joypad 2
    bool controllerKeyPressed(JoypadId idController, ControllerKey key);

    void requestStop();
    void switchDrawSprite();
    void switchInfoDisplayMode();

    bool getDrawSprite();
    int getInfoDisplayMode();

    bool isStopRequested();

protected:
    bool eventFilter(QObject* obj, QEvent* event);

private:
    Qt::Key _userKeys[6];
    Debugger* _debugger;
    Graphics* _graphics;

    bool _controller[2][6];
    bool _drawSprite;
    bool _isStopRequested;

    int _infoDisplayMode;
};
