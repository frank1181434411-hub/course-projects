#ifndef SHAPE_H
#define SHAPE_H
#include<QPoint>
#include<QColor>
#include<QPainter>
#include<QString>
class Shape
{
public:
    Shape(QPoint startPoint,QPoint endPoint,QColor color,int penWidth);
    virtual ~Shape()=default;

    virtual void draw(QPainter& painter) const=0;
    virtual QString typeName() const=0;
    virtual QString saveText() const=0;
    virtual bool contains(const QPoint& point) const=0;

    QPoint getStartPoint() const{return startPoint;}
    QPoint getEndPoint() const{return endPoint;}
    QColor getColor() const{return color;}
    int getPenWidth() const{return penWidth;}

protected:
    QPoint startPoint;
    QPoint endPoint;
    QColor color;
    int penWidth;
};
#endif //SHAPE_H
