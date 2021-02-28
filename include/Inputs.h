#pragma once

#include <string>
#include <variant>

#include <QObject>

#include "Debugger.h"
#include "Graphics.h"
#include "Singleton.h"
#include "definitions.h"

class Debugger;
class Graphics;

enum class ControllerType { kTypeKeyboard, kTypeMouse, kTypeJoystick };

constexpr int NUMBER_JOYSTICK_KEYS = 19;
enum class JoystickKeys {
	kNone,
	kStart,
	kSelect,
	kThumbLeftUp,
	kThumbLeftLeft,
	kThumbLeftDown,
	kThumbLeftRight,
	kThumbRightUp,
	kThumbRightLeft,
	kThumbRightDown,
	kThumbRightRight,
	kDpadUp,
	kDpadLeft,
	kDpadDown,
	kDpadRight,
	kButtonA,
	kButtonB,
	kButtonX,
	kButtonY
};

struct InputData {
	ControllerType type = ControllerType::kTypeKeyboard;
	std::variant<Qt::Key, JoystickKeys> data = Qt::Key::Key_unknown;
	int joystickId = -1;
};

class Inputs : public QObject, public Singleton<Inputs>
{
	Q_OBJECT
public:
	static constexpr double JOYSTICK_TRESHOLD = 0.4;
	static constexpr int NUMBER_JOYPAD = 2;
	static constexpr int NUMBER_KEYS = 6;
	enum JoypadId : size_t { kJoypad1 = 0, kJoypad2 = 1 };
	enum ControllerKey { CK_UP = 0, CK_DOWN = 1, CK_RIGHT = 2, CK_LEFT = 3, CK_FIREA = 4, CK_FIREB = 5 };

	static std::string keyToString(ControllerKey key);

	Inputs();
	virtual ~Inputs() = default;

	void aknowledgeStopRequest();

	void connect(JoypadId idController, bool value);
	// idController is 0 for Joypad 1 and 1 for Joypad 2
	bool controllerKeyPressed(JoypadId idController, ControllerKey key);

	bool getDrawSprite();
	int getInfoDisplayMode();
	InputData* getUserKeys(JoypadId idController);

	bool isConnected(JoypadId idController);
	bool isStopRequested();

	void requestStop();

	void setUserKeys(JoypadId idController, InputData inputDatas[NUMBER_KEYS]);

	void switchDrawSprite();
	void switchInfoDisplayMode();

protected:
	bool eventFilter(QObject* obj, QEvent* event);

private:
	Debugger* _debugger;
	bool _drawSprite;
	bool _controller[NUMBER_JOYPAD][NUMBER_KEYS];
	bool _controllerConnected[NUMBER_JOYPAD];
	ControllerType _controllerType[NUMBER_JOYPAD];
	std::vector<QMetaObject::Connection> _gamepadListeners;
	Graphics* _graphics;
	int _infoDisplayMode;
	bool _isStopRequested;
	InputData _userKeys[NUMBER_JOYPAD][NUMBER_KEYS];

	void addGamepadListeners(int iGamepadId, int indexJoypad);
	void refreshAllGamepadListeners();
};
