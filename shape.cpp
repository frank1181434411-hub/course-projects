#include"shape.h"
#include<cmath>

Shape::Shape(const QColor& color,int penWidth):color(color),penWidth(penWidth){}

QPointF Shape::controlPointAt(int) const
{
    return QPointF();
}

void Shape::moveControlPoint(int,const QPointF&)
{
}

int Shape::hitControlPoint(const QPointF& point,double radius) const
{
    int hitIndex=-1;
    double bestDistance=radius;
    for(int i=0;i<controlPointCount();++i)
    {
        QPointF controlPoint=controlPointAt(i);
        double dx=controlPoint.x()-point.x();
        double dy=controlPoint.y()-point.y();
        double distance=std::sqrt(dx*dx+dy*dy);
        if(distance<=bestDistance)
        {
            bestDistance=distance;
            hitIndex=i;
        }
    }
    return hitIndex;
}
