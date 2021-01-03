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
    
    static constexpr int NUMBER_JOYSTICK = 2;
    static constexpr int NUMBER_KEYS = 6;
    enum JoypadId : size_t { kJoypad1 = 0, kJoypad2 = 1 };
    enum ControllerKey { CK_UP = 0, CK_DOWN = 1, CK_RIGHT = 2, CK_LEFT = 3, CK_FIREA = 4, CK_FIREB = 5 };

    Inputs();
    virtual ~Inputs() = default;

    void aknowledgeStopRequest();

    void connect(JoypadId idController, bool value);
    // idController is 0 for Joypad 1 and 1 for Joypad 2
    bool controllerKeyPressed(JoypadId idController, ControllerKey key);

    Qt::Key* getControllerKeys(JoypadId idController);
    bool getDrawSprite();
    int getInfoDisplayMode();

    bool isConnected(JoypadId idController);
    bool isStopRequested();

    void requestStop();

    void setControllerKeys(JoypadId idController, Qt::Key keys[NUMBER_KEYS]);

    void switchDrawSprite();
    void switchInfoDisplayMode();

protected:
    bool eventFilter(QObject* obj, QEvent* event);

private:
    Debugger* _debugger;
    bool _drawSprite;
    bool _controller[NUMBER_JOYSTICK][NUMBER_KEYS];
    bool _controllerConnected[NUMBER_JOYSTICK];
    Graphics* _graphics;
    int _infoDisplayMode;
    bool _isStopRequested;
    Qt::Key _userKeys[NUMBER_JOYSTICK][NUMBER_KEYS];
};
