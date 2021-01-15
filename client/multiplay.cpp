#include "multiplay.h"

MultiPlay::MultiPlay()
{

}

void MultiPlay::GamePlay(const QString &addr, quint16 port)
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

    // Wait and send GAME_START
    if (socket->waitForConnected())
    {
        qDebug() << "Connected.";

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

        qDebug() << "Name: " << other_player_name;

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
            socket->waitForReadyRead();

            // Assign payload
            payload_len = socket->bytesAvailable();
            memcpy(payload_ptr, socket->readAll(), payload_len);
        }

        qDebug() << QByteArray((char*)payload_ptr, payload_len);

        // Header parsing
        hdr_parsing(payload_ptr, payload_len, &hdr);

        if (hdr.type == PUT) {
            qDebug() << "PUT packet received.";
        } else if (hdr.type == TURN) {
            qDebug() << "TURN packet received.";
        }

        // Pointer to end
        payload_ptr += hdr.data_length + PROTOCOL_HEADER_SIZE;
        payload_len -= hdr.data_length + PROTOCOL_HEADER_SIZE;
    }

    // Disconnect
    socket->close();
}
