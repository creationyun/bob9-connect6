#include "boardscene.h"

BoardScene::BoardScene(QObject *parent) : QGraphicsScene(parent)
{
    layable = false;
    //playerBrush.setColor(Qt::white);
    //playerBrush.setStyle(Qt::SolidPattern);
    //outlinePen.setColor(Qt::black);
}

void BoardScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (!layable) return;

    QPointF p = event->scenePos();

    p.setX(int((p.x()+12.5) / 25) * 25);
    p.setY(int((p.y()+12.5) / 25) * 25);

    //qDebug() << p.x()/25 << p.y()/25;

    //addEllipse(p.x()-12.5, p.y()-12.5, 25, 25, outlinePen, playerBrush);
    emit clickedBoard(uint8_t(p.x()/25), uint8_t(p.y()/25));
}
