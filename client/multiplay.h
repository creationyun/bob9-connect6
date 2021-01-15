#ifndef MULTIPLAY_H
#define MULTIPLAY_H

#include <QTcpSocket>
#include "../connect6_protocol/connect6_protocol.h"

class MultiPlay : public QObject
{
public:
    MultiPlay();
    void GamePlay(const QString &addr, quint16 port);
};

#endif // MULTIPLAY_H
