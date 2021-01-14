#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Set scene in Connect6 game board graphic view
    scene = new QGraphicsScene(this);
    ui->gvConnect6Board->setScene(scene);

    // Use brush with board color and pen
    QBrush boardBrush(QColor(220, 179, 92));
    QPen outlinePen(Qt::black);
    outlinePen.setWidth(1);

    // Draw whole board
    scene->addRect(-12.5, -12.5, 500-25, 500-25, outlinePen, boardBrush);

    // Draw blocks (19 x 19)
    for (int i = 0; i < BOARD_SIZE - 1; i++) {
        for (int j = 0; j < BOARD_SIZE - 1; j++) {
            scene->addRect(25*i, 25*j, 25, 25, outlinePen, boardBrush);
        }
    }
}

MainWindow::~MainWindow()
{
    delete ui;
    delete scene;
}

void MainWindow::on_pbSinglePlayButton_clicked()
{

}

void MainWindow::on_pbMultiPlayButton_clicked()
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
        socket->waitForReadyRead();

        qDebug() << "Reading: " << socket->bytesAvailable();

        qDebug() << socket->readAll();

        // Disconnect
        socket->close();
    }
    else
    {
        qDebug() << "Not connected.";
    }
}
