#ifndef MULTIPLAY_H
#define MULTIPLAY_H

#include <QTcpSocket>
#include <QGraphicsScene>
#include <QTextEdit>
#include <QThread>
#include "../connect6_protocol/connect6_protocol.h"

class MultiPlay : public QThread
{
    Q_OBJECT

public:
    MultiPlay();
    MultiPlay(QGraphicsScene *scene, QTextEdit *msg);
    ~MultiPlay();
    void setAddr(QString addr, quint16 port);
    void run();
    void PuttingStones(uint8_t *xy);

signals:
    void appendMsg(const QString&);
    void showStoneSignal(qreal, qreal, qreal, qreal, const QPen &, const QBrush &);

private slots:
    void showStone(qreal, qreal, qreal, qreal, const QPen &, const QBrush &);

private:
    QString addr;
    quint16 port;
    QGraphicsScene *scene;
    QTextEdit *msg;
};

#endif // MULTIPLAY_H
