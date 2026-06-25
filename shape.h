#ifndef SHAPE_H
#define SHAPE_H
#include<QColor>
#include<QPainter>
#include<QPointF>
#include<QString>

class Shape
{
public:
    Shape(const QColor& color,int penWidth);
    virtual ~Shape()=default;

    virtual void draw(QPainter& painter) const=0;
    virtual QString typeName() const=0;
    virtual int controlPointCount() const{return 0;}
    virtual QPointF controlPointAt(int index) const;
    virtual void moveControlPoint(int index,const QPointF& point);
    virtual int hitControlPoint(const QPointF& point,double radius) const;

    QColor getColor() const{return color;}
    int getPenWidth() const{return penWidth;}

protected:
    QColor color;
    int penWidth;
};

#endif //SHAPE_H
