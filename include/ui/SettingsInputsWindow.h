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
    SettingsInputsWindow(QWidget *parent = nullptr);
    virtual ~SettingsInputsWindow();

signals:
    void signal_captureTerminated(Qt::Key);

protected:
    virtual bool eventFilter(QObject* obj, QEvent* event) override;

private:
    Qt::Key _controllerKeys[Inputs::NUMBER_JOYSTICK][Inputs::NUMBER_KEYS];
    Ui::SettingsInputsWindow *ui;

    void captureKey(Inputs::JoypadId idController, Inputs::ControllerKey key, QPushButton* iPushButton);
    QPushButton* getNextButton(Inputs::JoypadId idController, Inputs::ControllerKey key, QPushButton* iPushButton);
    void loadInputsGui();
    virtual void keyPressEvent(QKeyEvent* e) override;
    QString keyToQstring(Qt::Key iKey);
    std::string keyToString(Qt::Key iKey);

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
};
