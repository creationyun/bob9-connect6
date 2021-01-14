#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_pbGameStartButton_clicked()
{
    bool ok;
    QString addr = QInputDialog::getText(this, tr("Connect to Server"),
                                            tr("Game Server Address:"), QLineEdit::Normal,
                                            tr(""), &ok);

    // When clicked cancel button or addr is empty
    if (!ok || addr.isEmpty())
    {
        return;
    }

    // TCP Socket variables for connection
    QTcpSocket *socket = new QTcpSocket(this);
    GameStartData gsd = {
        0x00, 9, "Creation"
    };
    unsigned char payload[1025];
    size_t payload_len;

    // Make GAME_START payload for starting
    make_game_start_payload(payload, 1024, &payload_len, 0x00, gsd);

    // Connect
    socket->connectToHost(addr, PORT);

    // Wait and send
    if (socket->waitForConnected())
    {
        qDebug() << "Connected.";

        // Send
        socket->write((const char *)payload, payload_len);
        //socket->waitForReadyRead();

        //qDebug() << "Reading: " << socket->bytesAvailable();

        //qDebug() << socket->readAll();

        // Disconnect
        socket->close();
    }
    else
    {
        qDebug() << "Not connected.";
    }
}
