#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QTcpSocket>
#include <QMessageBox>
#include <QTimer>

QT_BEGIN_NAMESPACE
namespace Ui {
class Widget;
}
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

    QTcpSocket* socket;
    QTimer timer;
    void socketConnected();
    void socketDisconnected();
    void socketError();
    void readyRead();
    void timerTimeout();
    QString buf = "";

private slots:
    void on_connectBtn_clicked();

    void on_startGameButton_clicked();

    void on_sendBtn_clicked();

private:
    Ui::Widget *ui;
};
#endif // WIDGET_H
