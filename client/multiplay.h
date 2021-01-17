#ifndef MULTIPLAY_H
#define MULTIPLAY_H

#include <QTcpSocket>
#include <QGraphicsScene>
#include <QTextEdit>
#include <QThread>
#include "../connect6_protocol/connect6_protocol.h"
#include "boardscene.h"

class MultiPlay : public QObject
{
    Q_OBJECT

public:
    MultiPlay();
    MultiPlay(BoardScene *scene, QTextEdit *msg);
    ~MultiPlay();
    void setAddr(QString addr, quint16 port);
    void play();
    void requestToSendPUT();

private slots:
    void gameStart();
    void readyPacketRead();
    void clickedBoard(uint8_t x, uint8_t y);

private:
    QTcpSocket *socket;
    QString addr;
    quint16 port;
    BoardScene *scene;
    QTextEdit *msg;

    enum GameState {
        GAME_NOT_STARTED,
        GAME_STARTED
    } state;

    bool layedBoard[BOARD_SIZE][BOARD_SIZE];
    unsigned char payload[1025];
    unsigned char *payload_ptr = payload;
    size_t payload_len;
    int player_num;
    QString other_player_name;

    size_t countInLayedStone;
    uint8_t layedStoneXY[4];
};

#endif // MULTIPLAY_H
