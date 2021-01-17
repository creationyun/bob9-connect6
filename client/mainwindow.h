#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QInputDialog>
#include <QGraphicsScene>
#include "multiplay.h"
#include "boardscene.h"

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
    void on_pbSinglePlayButton_clicked();

    void on_pbMultiPlayButton_clicked();

private:
    Ui::MainWindow *ui;

    BoardScene *scene;
};
#endif // MAINWINDOW_H
