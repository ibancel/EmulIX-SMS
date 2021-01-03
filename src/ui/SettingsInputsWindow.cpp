#include "ui/SettingsInputsWindow.h"
#include "ui_SettingsInputsWindow.h"

#include <QDialog>
#include <QtGamepad/QGamepad>
#include <QMessageBox>
#include <QTimer>

#include "Inputs.h"

SettingsInputsWindow::SettingsInputsWindow(QWidget *parent)
    : QDialog{parent},
      _controllerKeys{ },
      ui(new Ui::SettingsInputsWindow)
{
    ui->setupUi(this);
    setWindowTitle("Input Settings");

    connect(this, &QDialog::accepted, [&](){
        if(ui->comboBox_Player1->currentIndex() > 0) {
            Inputs::Instance()->setControllerKeys(Inputs::kJoypad1, _controllerKeys[Inputs::kJoypad1]);
        } else {
            Qt::Key emptyKeys[]{Qt::Key_unknown, Qt::Key_unknown, Qt::Key_unknown, Qt::Key_unknown, Qt::Key_unknown, Qt::Key_unknown};
            Inputs::Instance()->setControllerKeys(Inputs::kJoypad1, emptyKeys);
        }

        if(ui->comboBox_Player2->currentIndex() > 0) {
            Inputs::Instance()->setControllerKeys(Inputs::kJoypad2, _controllerKeys[Inputs::kJoypad2]);
        } else {
            Qt::Key emptyKeys[]{Qt::Key_unknown, Qt::Key_unknown, Qt::Key_unknown, Qt::Key_unknown, Qt::Key_unknown, Qt::Key_unknown};
            Inputs::Instance()->setControllerKeys(Inputs::kJoypad2, emptyKeys);
        }
    });

    loadInputsGui();

    {
        // Actual values player 1
        ui->comboBox_Player1->setCurrentIndex(Inputs::Instance()->isConnected(Inputs::kJoypad1) ? 1 : 0);
        Qt::Key* keys = Inputs::Instance()->getControllerKeys(Inputs::kJoypad1);
        ui->pushButton_Up_Player1->setText(keyToQstring(keys[Inputs::ControllerKey::CK_UP]));
        ui->pushButton_Left_Player1->setText(keyToQstring(keys[Inputs::ControllerKey::CK_LEFT]));
        ui->pushButton_Down_Player1->setText(keyToQstring(keys[Inputs::ControllerKey::CK_DOWN]));
        ui->pushButton_Right_Player1->setText(keyToQstring(keys[Inputs::ControllerKey::CK_RIGHT]));
        ui->pushButton_A_Player1->setText(keyToQstring(keys[Inputs::ControllerKey::CK_FIREA]));
        ui->pushButton_B_Player1->setText(keyToQstring(keys[Inputs::ControllerKey::CK_FIREB]));
        memcpy(&(_controllerKeys[Inputs::kJoypad1]), keys, Inputs::NUMBER_KEYS*sizeof(Qt::Key));
    }

    {
        // Actual values player 2
        ui->comboBox_Player2->setCurrentIndex(Inputs::Instance()->isConnected(Inputs::kJoypad2) ? 1 : 0);
        Qt::Key* keys = Inputs::Instance()->getControllerKeys(Inputs::kJoypad2);
        ui->pushButton_Up_Player2->setText(keyToQstring(keys[Inputs::ControllerKey::CK_UP]));
        ui->pushButton_Left_Player2->setText(keyToQstring(keys[Inputs::ControllerKey::CK_LEFT]));
        ui->pushButton_Down_Player2->setText(keyToQstring(keys[Inputs::ControllerKey::CK_DOWN]));
        ui->pushButton_Right_Player2->setText(keyToQstring(keys[Inputs::ControllerKey::CK_RIGHT]));
        ui->pushButton_A_Player2->setText(keyToQstring(keys[Inputs::ControllerKey::CK_FIREA]));
        ui->pushButton_B_Player2->setText(keyToQstring(keys[Inputs::ControllerKey::CK_FIREB]));
        memcpy(&(_controllerKeys[Inputs::kJoypad2]), keys, Inputs::NUMBER_KEYS*sizeof(Qt::Key));
    }
}

SettingsInputsWindow::~SettingsInputsWindow()
{

}

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

void SettingsInputsWindow::captureKey(Inputs::JoypadId idController, Inputs::ControllerKey idKey, QPushButton* iPushButton)
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
    QPushButton* nextPushButton = nullptr;

    setEnabled(false);

    do {
        QTimer timer;
        timer.setSingleShot(true);
        QEventLoop loop;
        connect( &timer, &QTimer::timeout, &loop, &QEventLoop::quit );
        connect(this, &SettingsInputsWindow::signal_captureTerminated, [&](Qt::Key key){
            currentPushButton->setText(keyToQstring(key));
            _controllerKeys[idController][idKey] = key;
            if(key != Qt::Key_unknown && key != 0 && idKey < Inputs::NUMBER_KEYS-1) {
                nextPushButton = getNextButton(idController, idKey, currentPushButton);
            } else{
                nextPushButton = nullptr;
            }
            loop.exit();
        });

        QString styleSheet = currentPushButton->styleSheet();
        currentPushButton->setStyleSheet("color:red;");
        installEventFilter(this);
        timer.start(5000);
        loop.exec();
        removeEventFilter(this);
        currentPushButton->setStyleSheet(styleSheet);
        currentPushButton = nextPushButton;
        idKey = static_cast<Inputs::ControllerKey>(idKey+1);
    } while(currentPushButton);

    setEnabled(true);
}

QPushButton* SettingsInputsWindow::getNextButton(Inputs::JoypadId idController, Inputs::ControllerKey key, QPushButton* iPushButton)
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

void SettingsInputsWindow::keyPressEvent(QKeyEvent *e) {
    if(e->key() != Qt::Key_Escape) {
        QDialog::keyPressEvent(e);
    } else {

    }
}

QString SettingsInputsWindow::keyToQstring(Qt::Key iKey)
{
    if(iKey == Qt::Key_unknown || iKey == 0) {
        return "None";
    }


    return QKeySequence(iKey).toString();
}


std::string SettingsInputsWindow::keyToString(Qt::Key iKey)
{
    return keyToQstring(iKey).toStdString();
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
