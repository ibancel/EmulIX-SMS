#pragma once

#include <QDialog>
#include <QString>
#include <QWidget>

#include "Inputs.h"

QT_BEGIN_NAMESPACE
namespace Ui { class SettingsInputsWindow; }
QT_END_NAMESPACE

class SettingsInputsWindow : public QDialog
{
    Q_OBJECT
public:
    static constexpr int CaptureTimeout = 5000; // In milliseconds
    static constexpr Inputs::ControllerKey KeysOrder[Inputs::NUMBER_KEYS] { Inputs::CK_UP, Inputs::CK_LEFT, Inputs::CK_DOWN, Inputs::CK_RIGHT, Inputs::CK_FIREA, Inputs::CK_FIREB };

    SettingsInputsWindow(QWidget *parent = nullptr);
    virtual ~SettingsInputsWindow();

signals:
    void signal_captureTerminated(Qt::Key);
    void signal_updateProgressBar(float value);

protected:
    virtual bool eventFilter(QObject* obj, QEvent* event) override;

private:
    bool _axisNeedReset[4]; // 0: left Y, 1: left X, 2: right Y, 3: right X

    InputData _userKeys[Inputs::NUMBER_JOYPAD][Inputs::NUMBER_KEYS];
    Ui::SettingsInputsWindow *ui;
    void captureKey(Inputs::JoypadId idController, Inputs::ControllerKey key, QPushButton* iPushButton);
    std::tuple<bool, Inputs::ControllerKey> findNextKey(Inputs::ControllerKey iKey);
    QPushButton* getNextButton(Inputs::ControllerKey key, QPushButton* iPushButton);
    void loadInputsGui();
    QString keyDataToQString(const InputData& iKeyData);
    std::string keyDataToString(const InputData& iKeyData);
    std::string keyJoystickToString(JoystickKeys iKeyJoystick);
    virtual void keyPressEvent(QKeyEvent* e) override;
    QString keyQtToQString(Qt::Key iKey);
    void printUserKeys();
    void setEnabledCaptureMode(bool inCapture);


private slots:
    void on_comboBox_Player1_currentIndexChanged(int index);
    void on_comboBox_Player2_currentIndexChanged(int index);
    void on_pushButton_Up_Player1_clicked();
    void on_pushButton_Left_Player1_clicked();
    void on_pushButton_Down_Player1_clicked();
    void on_pushButton_Right_Player1_clicked();
    void on_pushButton_A_Player1_clicked();
    void on_pushButton_B_Player1_clicked();
    void on_pushButton_Up_Player2_clicked();
    void on_pushButton_Left_Player2_clicked();
    void on_pushButton_Down_Player2_clicked();
    void on_pushButton_Right_Player2_clicked();
    void on_pushButton_A_Player2_clicked();
    void on_pushButton_B_Player2_clicked();
    void slot_updateProgressBar(int value);
};
