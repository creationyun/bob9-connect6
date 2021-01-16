#include "multiplay.h"

MultiPlay::MultiPlay()
{
    scene = nullptr;
    msg = nullptr;
}

MultiPlay::MultiPlay(QGraphicsScene *scene, QTextEdit *msg)
{
    this->scene = scene;
    this->msg = msg;

    connect(this, SIGNAL(appendMsg(const QString &)), msg, SLOT(append(const QString &)));
    connect(this, SIGNAL(showStoneSignal(qreal, qreal, qreal, qreal, const QPen &, const QBrush &)),
            this, SLOT(showStone(qreal, qreal, qreal, qreal, const QPen &, const QBrush &)));
}

MultiPlay::~MultiPlay()
{

}

void MultiPlay::showStone(qreal x, qreal y, qreal w, qreal h, const QPen &p, const QBrush &b)
{
    scene->addEllipse(x, y, w, h, p, b);
}

void MultiPlay::setAddr(QString addr, quint16 port)
{
    this->addr = addr;
    this->port = port;
}

void MultiPlay::run()
{
    // TCP Socket variables for connection
    QTcpSocket *socket = new QTcpSocket(this);
    GameStartData snd_gsd = {
        0x00, 9, "Creation"
    };
    unsigned char payload[1025];
    unsigned char *payload_ptr = payload;
    size_t payload_len;

    Connect6ProtocolHdr hdr;
    GameStartData rcv_gsd;
    int player_num;
    QString other_player_name;

    // Make GAME_START payload for starting
    make_game_start_payload(payload, 1024, &payload_len, 0x00, snd_gsd);

    // Connect
    socket->connectToHost(addr, port);

    emit appendMsg(tr("Connecting to ") + addr + ":" + QString::number(port) + " ...");

    // Wait and send GAME_START
    if (socket->waitForConnected())
    {
        emit appendMsg(tr("Connected."));

        // Send GAME_START
        socket->write((const char *)payload_ptr, payload_len);

        // Wait to receive packet from server
        socket->waitForReadyRead();

        // Assign payload
        payload_len = socket->bytesAvailable();
        memcpy(payload_ptr, socket->readAll(), payload_len);

        qDebug() << QByteArray((char*)payload_ptr, payload_len);

        // Header parsing
        hdr_parsing(payload_ptr, payload_len, &hdr);

        // Not GAME_START -> exit
        if (hdr.type != GAME_START) return;

        // Get PlayerNum
        player_num = hdr.player_num;

        qDebug() << "GAME_START packet received. PlayerNum: " << player_num;

        // Pointer to data field
        payload_ptr += PROTOCOL_HEADER_SIZE;
        payload_len -= PROTOCOL_HEADER_SIZE;

        // GAME_START data parsing
        qDebug() << game_start_data_parsing(payload_ptr, payload_len, &rcv_gsd);

        qDebug() << rcv_gsd.req_res_flag;
        qDebug() << rcv_gsd.name_length;

        // Get other player's name
        other_player_name = QString::fromUtf8(rcv_gsd.name, rcv_gsd.name_length-1);

        emit appendMsg(tr("Other Player Name: ") + other_player_name);

        // Pointer to end
        payload_ptr += hdr.data_length;
        payload_len -= hdr.data_length;
    }
    else
    {
        qDebug() << "Not connected.";
        return;
    }

    // Play game
    while (true) {
        // If no remained payload
        if (payload_len == 0) {
            payload_ptr = payload;
            if (!socket->waitForReadyRead()) break;

            // Assign payload
            payload_len = socket->bytesAvailable();
            memcpy(payload_ptr, socket->readAll(), payload_len);
        }

        qDebug() << QByteArray((char*)payload_ptr, payload_len);

        // Header parsing
        hdr_parsing(payload_ptr, payload_len, &hdr);

        QBrush playerBrush(hdr.player_num == 1 ? Qt::black : Qt::white);
        QPen outlinePen(Qt::black);

        // Pointer to data field
        payload_ptr += PROTOCOL_HEADER_SIZE;
        payload_len -= PROTOCOL_HEADER_SIZE;

        PutTurnData rcv_ptd, snd_ptd;

        if (hdr.type == PUT) {
            qDebug() << "PUT packet received.";

            qDebug() << put_turn_data_parsing(payload_ptr, payload_len, &rcv_ptd);
            qDebug() << "rcv_ptd.coord_num =" << rcv_ptd.coord_num;
            qDebug() << "rcv_ptd.x1 =" << rcv_ptd.xy[0];
            qDebug() << "rcv_ptd.y1 =" << rcv_ptd.xy[1];

            for (int i = 0; i < rcv_ptd.coord_num; i++) {
                emit showStoneSignal(25*rcv_ptd.xy[2*i]-12.5, 25*rcv_ptd.xy[2*i+1]-12.5, 25, 25, outlinePen, playerBrush);
            }
        } else if (hdr.type == TURN) {
            qDebug() << "TURN packet received.";

            qDebug() << put_turn_data_parsing(payload_ptr, payload_len, &rcv_ptd);
            qDebug() << "rcv_ptd.coord_num =" << rcv_ptd.coord_num;
            qDebug() << "rcv_ptd.x1 =" << rcv_ptd.xy[0];
            qDebug() << "rcv_ptd.y1 =" << rcv_ptd.xy[1];

            for (int i = 0; i < rcv_ptd.coord_num; i++) {
                emit showStoneSignal(25*rcv_ptd.xy[2*i]-12.5, 25*rcv_ptd.xy[2*i+1]-12.5, 25, 25, outlinePen, playerBrush);
            }
        } else {
            qDebug() << "Illegal packet received.";
        }

        // Pointer to end
        payload_ptr += hdr.data_length;
        payload_len -= hdr.data_length;
    }

    // Disconnect
    socket->close();
}

void MultiPlay::PuttingStones(uint8_t *xy)
{

}
