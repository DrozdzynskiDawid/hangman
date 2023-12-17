#include "widget.h"
#include "./ui_widget.h"
#include "../config/config.h"
#include "../Message.h"
#include <string.h>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    socket = new QTcpSocket(this);
    connect(ui->connectBtn, &QPushButton::clicked, this, &Widget::on_connectBtn_clicked);
    connect(socket, &QTcpSocket::connected, this, &Widget::socketConnected);
    connect(socket, &QTcpSocket::disconnected, this, &Widget::socketDisconnected);
    //connect(socket, &QTcpSocket::errorOccurred, this, &Widget::socketError);
    connect(socket, &QTcpSocket::readyRead, this, &Widget::readyRead);
    ui->wordText->setText("YOUR WORD:");
    ui->wordText->setAlignment(Qt::AlignCenter);
    ui->startGameButton->setDisabled(true);
    ui->letterGroup->setDisabled(true);
}

Widget::~Widget()
{
    delete ui;
}

void Widget::on_connectBtn_clicked()
{
    QString port = SERVER_PORT;
    socket->connectToHost(SERVER_ADDRESS,port.toInt());
}

void Widget::socketConnected()
{
    QMessageBox* msgBox = new QMessageBox(this);
    msgBox->setWindowTitle("Connected!");
    msgBox->setText("Connected to server!");
    msgBox->exec();

    QString nick = ui->nickLineEdit->text().trimmed();
    Message message;
    message.setCmd("NICK");
    message.setMsg(nick.toStdString());
    QString serialized = QString::fromStdString(message.serialize());
    socket->write(serialized.toUtf8());

    ui->connectGroup->setDisabled(true);
    ui->startGameButton->setDisabled(false);
}

void Widget::socketDisconnected()
{
    ui->msgsTextEdit->append("disconnected");
}

// void Widget::socketError()
// {
//     ui->msgsTextEdit->append("error");
// }

void Widget::readyRead()
{
    QString txt = "";
    do {
        txt += socket->readAll();
    }
    while (txt.back() != '\n');

    // creating message object
    Message message;
    std::string text = txt.toStdString();
    message.deserialize(text);

    // diffrent commands handling
    if (message.getCmd() == "I") {
        ui->msgsTextEdit->append(QString::fromStdString(message.getMsg()));
    }
    else if (message.getCmd() == "W") {
        QString word = QString::fromStdString(message.getMsg());

        // adding spaces for better display
        QString displayWord = "";
        for (int i=0; i<word.length(); i++) {
            displayWord += word[i];
            displayWord += " ";
        }
        ui->wordText->setText("YOUR WORD:\n\n\n");
        ui->wordText->setAlignment(Qt::AlignCenter);
        ui->wordText->append(displayWord);
        ui->letterGroup->setEnabled(true);
        ui->startGameButton->setDisabled(true);
    }
    else if (message.getCmd() == "E") {
        ui->letterGroup->setDisabled(true);
    }
}

void Widget::on_startGameButton_clicked()
{
    Message message;
    message.setCmd("START");
    message.setMsg("Start game clicked");
    QString serialized = QString::fromStdString(message.serialize());
    socket->write(serialized.toUtf8());
}


void Widget::on_sendBtn_clicked()
{
    if (ui->letterLineEdit->text().length() == 1) {
        QChar letter = ui->letterLineEdit->text().trimmed()[0];
        if (letter.isLetter()) {
            letter = letter.toLower();
            QString msg;
            msg = letter;
            Message message;
            message.setCmd("LETTER");
            message.setMsg(msg.toStdString());
            QString serialized = QString::fromStdString(message.serialize());
            socket->write(serialized.toUtf8());
            ui->letterLineEdit->setText("");
        }
    }
}

