#include "ui/SettingsInputsWindow.h"
#include "ui_SettingsInputsWindow.h"

#include <cmath>

#include <QDialog>
#include <QMessageBox>
#include <QThread>
#include <QTimer>

#if SUPPORT_GAMEPAD
#include <QtGamepad/QGamepad>
#include <QtGamepad/QGamepadManager>
#endif

#include "Inputs.h"

SettingsInputsWindow::SettingsInputsWindow(QWidget* parent)
	: QDialog { parent }, _axisNeedReset {}, _userKeys {}, ui(new Ui::SettingsInputsWindow)
{
	ui->setupUi(this);
	setWindowTitle("Input Settings");
	ui->progressBar_capture->setVisible(false);
	ui->label_capture->setVisible(false);

	ui->progressBar_capture->setMaximum(CaptureTimeout);

	connect(this, &QDialog::accepted, [&]() {
		if(ui->comboBox_Player1->currentIndex() > 0) {
			Inputs::Instance()->setUserKeys(Inputs::kJoypad1, _userKeys[Inputs::kJoypad1]);
		} else {
			InputData emptyKeys[Inputs::NUMBER_KEYS] {};
			Inputs::Instance()->setUserKeys(Inputs::kJoypad1, emptyKeys);
		}

		if(ui->comboBox_Player2->currentIndex() > 0) {
			Inputs::Instance()->setUserKeys(Inputs::kJoypad2, _userKeys[Inputs::kJoypad2]);
		} else {
			InputData emptyKeys[Inputs::NUMBER_KEYS] {};
			Inputs::Instance()->setUserKeys(Inputs::kJoypad2, emptyKeys);
		}
	});

#if SUPPORT_GAMEPAD
	auto gamepads = QGamepadManager::instance()->connectedGamepads();
	for(int i = 0; i < gamepads.size(); i++) {
		QGamepad* aGamepad = new QGamepad(gamepads[i], this);
		ui->comboBox_Player1->addItem(aGamepad->name(), QVariant(gamepads[i]));
		ui->comboBox_Player2->addItem(aGamepad->name(), QVariant(gamepads[i]));
	}
#endif

	loadInputsGui();

	{
		// Actual values player 1
		ui->comboBox_Player1->setCurrentIndex(Inputs::Instance()->isConnected(Inputs::kJoypad1) ? 1 : 0);
		InputData* keys = Inputs::Instance()->getUserKeys(Inputs::kJoypad1);
		ui->pushButton_Up_Player1->setText(keyDataToQString(keys[Inputs::ControllerKey::CK_UP]));
		ui->pushButton_Left_Player1->setText(keyDataToQString(keys[Inputs::ControllerKey::CK_LEFT]));
		ui->pushButton_Down_Player1->setText(keyDataToQString(keys[Inputs::ControllerKey::CK_DOWN]));
		ui->pushButton_Right_Player1->setText(keyDataToQString(keys[Inputs::ControllerKey::CK_RIGHT]));
		ui->pushButton_A_Player1->setText(keyDataToQString(keys[Inputs::ControllerKey::CK_FIREA]));
		ui->pushButton_B_Player1->setText(keyDataToQString(keys[Inputs::ControllerKey::CK_FIREB]));
		memcpy(&(_userKeys[Inputs::kJoypad1]), keys, Inputs::NUMBER_KEYS * sizeof(InputData));
	}

	{
		// Actual values player 2
		ui->comboBox_Player2->setCurrentIndex(Inputs::Instance()->isConnected(Inputs::kJoypad2) ? 1 : 0);
		InputData* keys = Inputs::Instance()->getUserKeys(Inputs::kJoypad2);
		ui->pushButton_Up_Player2->setText(keyDataToQString(keys[Inputs::ControllerKey::CK_UP]));
		ui->pushButton_Left_Player2->setText(keyDataToQString(keys[Inputs::ControllerKey::CK_LEFT]));
		ui->pushButton_Down_Player2->setText(keyDataToQString(keys[Inputs::ControllerKey::CK_DOWN]));
		ui->pushButton_Right_Player2->setText(keyDataToQString(keys[Inputs::ControllerKey::CK_RIGHT]));
		ui->pushButton_A_Player2->setText(keyDataToQString(keys[Inputs::ControllerKey::CK_FIREA]));
		ui->pushButton_B_Player2->setText(keyDataToQString(keys[Inputs::ControllerKey::CK_FIREB]));
		memcpy(&(_userKeys[Inputs::kJoypad2]), keys, Inputs::NUMBER_KEYS * sizeof(InputData));
	}
}

SettingsInputsWindow::~SettingsInputsWindow() { }

// Protected:

bool SettingsInputsWindow::eventFilter(QObject* obj, QEvent* event)
{
	if(event->type() == QEvent::KeyRelease) {
		QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
		if(!keyEvent->isAutoRepeat()) {
			Qt::Key keyPressed = static_cast<Qt::Key>(keyEvent->key());
			//            qDebug() << keyEvent->key() << " " << event->type() << " " << keyEvent->isAutoRepeat();
			emit(signal_captureTerminated(keyPressed));
		}
	}
	if(event->type() == QEvent::KeyPress) {
		QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
		if(!keyEvent->isAutoRepeat()) {
			Qt::Key keyPressed = static_cast<Qt::Key>(keyEvent->key());
			if(keyPressed == Qt::Key_Escape) {
				emit(signal_captureTerminated(Qt::Key_unknown));
				return true;
			}

			return true;
		}
	}

	return QObject::eventFilter(obj, event);
}

// Private:

void SettingsInputsWindow::captureKey(
	Inputs::JoypadId idController, Inputs::ControllerKey iKey, QPushButton* iPushButton)
{
	QList<QWidget*> focusChain;
	focusChain.push_back(nextInFocusChain());
	while(focusChain.back() != nullptr) {
		if(focusChain.back()->nextInFocusChain() == focusChain.front()) {
			break;
		}
		focusChain.push_back(focusChain.back()->nextInFocusChain());
	}

	QPushButton* currentPushButton = iPushButton;

	setEnabledCaptureMode(true);
	ui->progressBar_capture->setVisible(true);
	ui->label_capture->setVisible(true);

	do {
		ui->progressBar_capture->setValue(CaptureTimeout);
		ui->label_capture->setText(QString::fromStdString(Inputs::keyToString(iKey)));

		QList<QMetaObject::Connection> disconnectList;
		bool captureHandled = false;

		QTimer timer;
		timer.setSingleShot(true);
		QEventLoop loop;
		QThread* threadTimer = QThread::create([this, &timer, &loop]() {
			timer.start(CaptureTimeout);
			while(!loop.isRunning()) { }
			while(loop.isRunning() && timer.remainingTime() > 0) {
				emit(signal_updateProgressBar(timer.remainingTime()));
				update();
				std::this_thread::sleep_for(std::chrono::milliseconds(30));
				QCoreApplication::processEvents();
			}
			timer.stop();
		});
		timer.moveToThread(threadTimer);
		disconnectList.push_back(connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit, Qt::DirectConnection));
		disconnectList.push_back(connect(this, &SettingsInputsWindow::signal_updateProgressBar, this,
			&SettingsInputsWindow::slot_updateProgressBar));

		if((idController == Inputs::kJoypad1 && ui->comboBox_Player1->currentIndex() == 1)
			|| (idController == Inputs::kJoypad2 && ui->comboBox_Player2->currentIndex() == 1)) {
			disconnectList.push_back(
				connect(this, &SettingsInputsWindow::signal_captureTerminated, this, [&](Qt::Key key) {
					_userKeys[idController][iKey].type = ControllerType::kTypeKeyboard;
					_userKeys[idController][iKey].data = key;
					if(key != Qt::Key_unknown && key != 0) {
						captureHandled = true;
					}
					loop.exit();
				}));
		} else {
			disconnectList.push_back(connect(this, &SettingsInputsWindow::signal_captureTerminated, [&](Qt::Key key) {
				if(key == Qt::Key_unknown || key == 0) {
					loop.exit();
				}
			}));

#if SUPPORT_GAMEPAD
			std::shared_ptr<QGamepad> aGamepad;
			if(idController == Inputs::kJoypad1) {
				aGamepad = std::make_shared<QGamepad>(ui->comboBox_Player1->currentData().toInt(), this);
			} else if(idController == Inputs::kJoypad2) {
				aGamepad = std::make_shared<QGamepad>(ui->comboBox_Player2->currentData().toInt(), this);
			}
			if(!aGamepad) {
				throw std::domain_error("No defined QGamepad for this player controller");
			}
			disconnectList.push_back(
				connect(aGamepad.get(), &QGamepad::axisLeftXChanged, this, [&, aGamepad](double value) {
					if(!_axisNeedReset[1] && abs(value) > Inputs::JOYSTICK_TRESHOLD) {
						_userKeys[idController][iKey].type = ControllerType::kTypeJoystick;
						_userKeys[idController][iKey].data
							= value > 0 ? JoystickKeys::kThumbLeftRight : JoystickKeys::kThumbLeftLeft;
						_userKeys[idController][iKey].joystickId = aGamepad->deviceId();
						_axisNeedReset[1] = true;
						captureHandled = true;
						loop.exit();
					} else if(_axisNeedReset[1] && abs(value) <= Inputs::JOYSTICK_TRESHOLD * 0.5) {
						_axisNeedReset[1] = false;
					}
				}));
			disconnectList.push_back(
				connect(aGamepad.get(), &QGamepad::axisLeftYChanged, this, [&, aGamepad](double value) {
					if(!_axisNeedReset[0] && abs(value) > Inputs::JOYSTICK_TRESHOLD) {
						_userKeys[idController][iKey].type = ControllerType::kTypeJoystick;
						_userKeys[idController][iKey].data
							= value < 0 ? JoystickKeys::kThumbLeftUp : JoystickKeys::kThumbLeftDown;
						_userKeys[idController][iKey].joystickId = aGamepad->deviceId();
						_axisNeedReset[0] = true;
						captureHandled = true;
						loop.exit();
					} else if(_axisNeedReset[0] && abs(value) <= Inputs::JOYSTICK_TRESHOLD * 0.5) {
						_axisNeedReset[0] = false;
					}
				}));
			disconnectList.push_back(
				connect(aGamepad.get(), &QGamepad::axisRightXChanged, this, [&, aGamepad](double value) {
					if(!_axisNeedReset[3] && abs(value) > Inputs::JOYSTICK_TRESHOLD) {
						_userKeys[idController][iKey].type = ControllerType::kTypeJoystick;
						_userKeys[idController][iKey].data
							= value > 0 ? JoystickKeys::kThumbRightRight : JoystickKeys::kThumbRightLeft;
						_userKeys[idController][iKey].joystickId = aGamepad->deviceId();
						_axisNeedReset[3] = true;
						captureHandled = true;
						loop.exit();
					} else if(_axisNeedReset[3] && abs(value) <= Inputs::JOYSTICK_TRESHOLD * 0.5) {
						_axisNeedReset[3] = false;
					}
				}));
			disconnectList.push_back(
				connect(aGamepad.get(), &QGamepad::axisRightYChanged, this, [&, aGamepad](double value) {
					if(!_axisNeedReset[2] && abs(value) > Inputs::JOYSTICK_TRESHOLD) {
						_userKeys[idController][iKey].type = ControllerType::kTypeJoystick;
						_userKeys[idController][iKey].data
							= value < 0 ? JoystickKeys::kThumbRightUp : JoystickKeys::kThumbRightDown;
						_userKeys[idController][iKey].joystickId = aGamepad->deviceId();
						_axisNeedReset[2] = true;
						captureHandled = true;
						loop.exit();
					} else if(_axisNeedReset[2] && abs(value) <= Inputs::JOYSTICK_TRESHOLD * 0.5) {
						_axisNeedReset[2] = false;
					}
				}));

			disconnectList.push_back(
				connect(aGamepad.get(), &QGamepad::buttonUpChanged, this, [&, aGamepad](bool pressed) {
					if(pressed) {
						_userKeys[idController][iKey].type = ControllerType::kTypeJoystick;
						_userKeys[idController][iKey].data = JoystickKeys::kDpadUp;
						captureHandled = true;
						loop.exit();
					}
				}));
			disconnectList.push_back(
				connect(aGamepad.get(), &QGamepad::buttonLeftChanged, this, [&, aGamepad](bool pressed) {
					if(pressed) {
						_userKeys[idController][iKey].type = ControllerType::kTypeJoystick;
						_userKeys[idController][iKey].data = JoystickKeys::kDpadLeft;
						captureHandled = true;
						loop.exit();
					}
				}));
			disconnectList.push_back(
				connect(aGamepad.get(), &QGamepad::buttonDownChanged, this, [&, aGamepad](bool pressed) {
					if(pressed) {
						_userKeys[idController][iKey].type = ControllerType::kTypeJoystick;
						_userKeys[idController][iKey].data = JoystickKeys::kDpadDown;
						captureHandled = true;
						loop.exit();
					}
				}));
			disconnectList.push_back(
				connect(aGamepad.get(), &QGamepad::buttonRightChanged, this, [&, aGamepad](bool pressed) {
					if(pressed) {
						_userKeys[idController][iKey].type = ControllerType::kTypeJoystick;
						_userKeys[idController][iKey].data = JoystickKeys::kDpadRight;
						captureHandled = true;
						loop.exit();
					}
				}));

			disconnectList.push_back(
				connect(aGamepad.get(), &QGamepad::buttonAChanged, this, [&, aGamepad](bool pressed) {
					if(pressed) {
						_userKeys[idController][iKey].type = ControllerType::kTypeJoystick;
						_userKeys[idController][iKey].data = JoystickKeys::kButtonA;
						captureHandled = true;
						loop.exit();
					}
				}));
			disconnectList.push_back(
				connect(aGamepad.get(), &QGamepad::buttonBChanged, this, [&, aGamepad](bool pressed) {
					if(pressed) {
						_userKeys[idController][iKey].type = ControllerType::kTypeJoystick;
						_userKeys[idController][iKey].data = JoystickKeys::kButtonB;
						captureHandled = true;
						loop.exit();
					}
				}));
			disconnectList.push_back(
				connect(aGamepad.get(), &QGamepad::buttonXChanged, this, [&, aGamepad](bool pressed) {
					if(pressed) {
						_userKeys[idController][iKey].type = ControllerType::kTypeJoystick;
						_userKeys[idController][iKey].data = JoystickKeys::kButtonX;
						captureHandled = true;
						loop.exit();
					}
				}));
			disconnectList.push_back(
				connect(aGamepad.get(), &QGamepad::buttonYChanged, this, [&, aGamepad](bool pressed) {
					if(pressed) {
						_userKeys[idController][iKey].type = ControllerType::kTypeJoystick;
						_userKeys[idController][iKey].data = JoystickKeys::kButtonY;
						captureHandled = true;
						loop.exit();
					}
				}));
#endif
		}

		QString styleSheet = currentPushButton->styleSheet();
		currentPushButton->setStyleSheet("color:red;");
		installEventFilter(this);
		threadTimer->start();
		loop.exec();
		removeEventFilter(this);
		currentPushButton->setStyleSheet(styleSheet);
		threadTimer->wait();

		if(captureHandled) {
			currentPushButton->setText(keyDataToQString(_userKeys[idController][iKey]));
		}

		auto [hasNextKey, nextKey] = findNextKey(iKey);
		if(captureHandled && hasNextKey) {
			currentPushButton = getNextButton(iKey, currentPushButton);
			iKey = nextKey;
		} else {
			currentPushButton = nullptr;
		}

		for(auto disconnectElement : disconnectList) {
			disconnect(disconnectElement);
		}

	} while(currentPushButton);

	ui->progressBar_capture->setVisible(false);
	ui->label_capture->setVisible(false);
	setEnabledCaptureMode(false);
}

std::tuple<bool, Inputs::ControllerKey> SettingsInputsWindow::findNextKey(Inputs::ControllerKey iKey)
{
	int indexKey = -1;
	for(int i = 0; i < Inputs::NUMBER_KEYS - 1 && indexKey == -1; i++) {
		if(SettingsInputsWindow::KeysOrder[i] == iKey) {
			indexKey = i;
		}
	}

	if(indexKey == -1) {
		return { false, Inputs::CK_DOWN };
	}

	return { true, SettingsInputsWindow::KeysOrder[indexKey + 1] };
}

QPushButton* SettingsInputsWindow::getNextButton(Inputs::ControllerKey key, QPushButton* iPushButton)
{
	if(key >= Inputs::NUMBER_KEYS) {
		return nullptr;
	}
	QWidget* currentElement = iPushButton;
	QPushButton* nextPushButton = nullptr;
	while(!(nextPushButton = dynamic_cast<QPushButton*>(currentElement->nextInFocusChain()))) {
		currentElement = currentElement->nextInFocusChain();
	}
	return nextPushButton;
}

QString SettingsInputsWindow::keyDataToQString(const InputData& iKeyData)
{
	switch(iKeyData.type) {
		case ControllerType::kTypeKeyboard:
			return keyQtToQString(std::get<Qt::Key>(iKeyData.data));
			break;
		case ControllerType::kTypeJoystick:
			return QString::fromStdString(keyJoystickToString(std::get<JoystickKeys>(iKeyData.data)));
			break;
		default:
			return QString("None");
			break;
	}
}

std::string SettingsInputsWindow::keyDataToString(const InputData& iKeyData)
{
	return keyDataToQString(iKeyData).toStdString();
}

std::string SettingsInputsWindow::keyJoystickToString(JoystickKeys iKeyJoystick)
{
	switch(iKeyJoystick) {
		case JoystickKeys::kButtonA:
			return "Button A";
			break;
		case JoystickKeys::kButtonB:
			return "Button B";
			break;
		case JoystickKeys::kButtonX:
			return "Button X";
			break;
		case JoystickKeys::kButtonY:
			return "Button Y";
			break;
		case JoystickKeys::kDpadDown:
			return "DPad Down";
			break;
		case JoystickKeys::kDpadLeft:
			return "DPad Left";
			break;
		case JoystickKeys::kDpadRight:
			return "DPad Right";
			break;
		case JoystickKeys::kDpadUp:
			return "DPad Up";
			break;
		case JoystickKeys::kThumbLeftUp:
			return "Left Y+";
			break;
		case JoystickKeys::kThumbLeftLeft:
			return "Left X-";
			break;
		case JoystickKeys::kThumbLeftDown:
			return "Left Y-";
			break;
		case JoystickKeys::kThumbLeftRight:
			return "Left X+";
			break;
		case JoystickKeys::kThumbRightUp:
			return "Right Y+";
			break;
		case JoystickKeys::kThumbRightLeft:
			return "Right X-";
			break;
		case JoystickKeys::kThumbRightDown:
			return "Right Y-";
			break;
		case JoystickKeys::kThumbRightRight:
			return "Right X+";
			break;
		default:
			return "None";
			break;
	}
}

void SettingsInputsWindow::keyPressEvent(QKeyEvent* e)
{
	if(e->key() != Qt::Key_Escape) {
		QDialog::keyPressEvent(e);
	} else {
	}
}

QString SettingsInputsWindow::keyQtToQString(Qt::Key iKey)
{
	if(iKey == Qt::Key_unknown || iKey == 0) {
		return "None";
	}

	return QKeySequence(iKey).toString();
}

void SettingsInputsWindow::loadInputsGui()
{
	if(ui->comboBox_Player1->currentIndex() > 0) {
		ui->pushButton_Up_Player1->setEnabled(true);
		ui->pushButton_Left_Player1->setEnabled(true);
		ui->pushButton_Down_Player1->setEnabled(true);
		ui->pushButton_Right_Player1->setEnabled(true);
		ui->pushButton_A_Player1->setEnabled(true);
		ui->pushButton_B_Player1->setEnabled(true);
	} else {
		ui->pushButton_Up_Player1->setEnabled(false);
		ui->pushButton_Left_Player1->setEnabled(false);
		ui->pushButton_Down_Player1->setEnabled(false);
		ui->pushButton_Right_Player1->setEnabled(false);
		ui->pushButton_A_Player1->setEnabled(false);
		ui->pushButton_B_Player1->setEnabled(false);
	}

	if(ui->comboBox_Player2->currentIndex() > 0) {
		ui->pushButton_Up_Player2->setEnabled(true);
		ui->pushButton_Left_Player2->setEnabled(true);
		ui->pushButton_Down_Player2->setEnabled(true);
		ui->pushButton_Right_Player2->setEnabled(true);
		ui->pushButton_A_Player2->setEnabled(true);
		ui->pushButton_B_Player2->setEnabled(true);
	} else {
		ui->pushButton_Up_Player2->setEnabled(false);
		ui->pushButton_Left_Player2->setEnabled(false);
		ui->pushButton_Down_Player2->setEnabled(false);
		ui->pushButton_Right_Player2->setEnabled(false);
		ui->pushButton_A_Player2->setEnabled(false);
		ui->pushButton_B_Player2->setEnabled(false);
	}
}

void SettingsInputsWindow::printUserKeys()
{
	for(int indexJoystick = 0; indexJoystick < Inputs::NUMBER_JOYPAD; indexJoystick++) {
		std::cout << Inputs::Instance()->isConnected(static_cast<Inputs::JoypadId>(indexJoystick)) << ": ";
		for(int indexKey = 0; indexKey < Inputs::NUMBER_KEYS; indexKey++) {
			std::cout << keyDataToString(_userKeys[indexJoystick][indexKey]) << " | ";
		}
		std::cout << std::endl;
	}
}

void SettingsInputsWindow::setEnabledCaptureMode(bool inCapture)
{
	ui->groupBox_player1->setEnabled(!inCapture);
	ui->groupBox_player2->setEnabled(!inCapture);
	ui->layout_captureInfos->setEnabled(inCapture);
	ui->buttonBox->setEnabled(!inCapture);
}

// Slots:

void SettingsInputsWindow::on_comboBox_Player1_currentIndexChanged(int index)
{
	Inputs::Instance()->connect(Inputs::kJoypad1, (index > 0));
	loadInputsGui();
}

void SettingsInputsWindow::on_comboBox_Player2_currentIndexChanged(int index)
{
	Inputs::Instance()->connect(Inputs::kJoypad2, (index > 0));
	loadInputsGui();
}

void SettingsInputsWindow::on_pushButton_Up_Player1_clicked()
{
	captureKey(Inputs::kJoypad1, Inputs::CK_UP, ui->pushButton_Up_Player1);
}

void SettingsInputsWindow::on_pushButton_Up_Player2_clicked()
{
	captureKey(Inputs::kJoypad2, Inputs::CK_UP, ui->pushButton_Up_Player2);
}

void SettingsInputsWindow::on_pushButton_Left_Player1_clicked()
{
	captureKey(Inputs::kJoypad1, Inputs::CK_LEFT, ui->pushButton_Left_Player1);
}

void SettingsInputsWindow::on_pushButton_Left_Player2_clicked()
{
	captureKey(Inputs::kJoypad2, Inputs::CK_LEFT, ui->pushButton_Left_Player2);
}

void SettingsInputsWindow::on_pushButton_Down_Player1_clicked()
{
	captureKey(Inputs::kJoypad1, Inputs::CK_DOWN, ui->pushButton_Down_Player1);
}

void SettingsInputsWindow::on_pushButton_Down_Player2_clicked()
{
	captureKey(Inputs::kJoypad2, Inputs::CK_DOWN, ui->pushButton_Down_Player2);
}

void SettingsInputsWindow::on_pushButton_Right_Player1_clicked()
{
	captureKey(Inputs::kJoypad1, Inputs::CK_RIGHT, ui->pushButton_Right_Player1);
}

void SettingsInputsWindow::on_pushButton_Right_Player2_clicked()
{
	captureKey(Inputs::kJoypad2, Inputs::CK_RIGHT, ui->pushButton_Right_Player2);
}

void SettingsInputsWindow::on_pushButton_A_Player1_clicked()
{
	captureKey(Inputs::kJoypad1, Inputs::CK_FIREA, ui->pushButton_A_Player1);
}

void SettingsInputsWindow::on_pushButton_A_Player2_clicked()
{
	captureKey(Inputs::kJoypad2, Inputs::CK_FIREA, ui->pushButton_A_Player2);
}

void SettingsInputsWindow::on_pushButton_B_Player1_clicked()
{
	captureKey(Inputs::kJoypad1, Inputs::CK_FIREB, ui->pushButton_B_Player1);
}

void SettingsInputsWindow::on_pushButton_B_Player2_clicked()
{
	captureKey(Inputs::kJoypad2, Inputs::CK_FIREB, ui->pushButton_B_Player2);
}

void SettingsInputsWindow::slot_updateProgressBar(int value) { ui->progressBar_capture->setValue(value); }
