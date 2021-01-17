#ifndef BOARDSCENE_H
#define BOARDSCENE_H

#include <QDebug>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>

class BoardScene : public QGraphicsScene
{
    Q_OBJECT

public:
    BoardScene(QObject *parent = nullptr);
    void setLayableOn() { layable = true; }
    void setLayableOff() { layable = false; }
    //void setBrush(const QColor& c) { playerBrush.setColor(c); }
    //void setBrush(Qt::GlobalColor c) { playerBrush.setColor(c); }

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);

signals:
    void clickedBoard(uint8_t x, uint8_t y);

private:
    bool layable;
    //QBrush playerBrush;
    //QPen outlinePen;
};

#endif // BOARDSCENE_H
