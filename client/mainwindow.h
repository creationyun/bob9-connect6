#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QInputDialog>
#include <QTcpSocket>
#include "../connect6_protocol/connect6_protocol.h"

// Fixed Game Options
#define PORT 8089
#define MAX_PLAYER 2
#define TIMEOUT_SECONDS 30

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_pbGameStartButton_clicked();

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
