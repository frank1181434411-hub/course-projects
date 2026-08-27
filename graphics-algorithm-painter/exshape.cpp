#include"exshape.h"
#include<cmath>
#include<algorithm>
namespace
{
double pointToSegmentDistance(const QPoint& point,const QPoint& a,const QPoint& b)
{
    double dx=b.x()-a.x();
    double dy=b.y()-a.y();
    if(dx==0 && dy==0)
    {
        double px=point.x()-a.x();
        double py=point.y()-a.y();
        return std::sqrt(px*px+py*py);
    }
    double t=((point.x()-a.x())*dx+(point.y()-a.y())*dy)/(dx*dx+dy*dy);
    t=std::max(0.0,std::min(1.0,t));
    double x=a.x()+t*dx;
    double y=a.y()+t*dy;
    double px=point.x()-x;
    double py=point.y()-y;
    return std::sqrt(px*px+py*py);
}
bool nearSegment(const QPoint& point,const QPoint& a,const QPoint& b,int width) {return pointToSegmentDistance(point,a,b)<=std::max(6,width+3);}
}
//LineShape
LineShape::LineShape(QPoint startPoint,QPoint endPoint,QColor color,int penWidth)
    :Shape(startPoint,endPoint,color,penWidth){}
void LineShape::draw(QPainter& painter) const
{
    painter.setPen(QPen(color,penWidth));
    painter.drawLine(startPoint,endPoint);
}
QString LineShape::saveText() const
{
    return QString("Line %1 %2 %3 %4 %5 %6 %7 %8")
        .arg(startPoint.x()).arg(startPoint.y())
        .arg(endPoint.x()).arg(endPoint.y())
        .arg(color.red()).arg(color.green()).arg(color.blue())
        .arg(penWidth);
}
bool LineShape::contains(const QPoint& point) const {return nearSegment(point,startPoint,endPoint,penWidth);}
//RectShape
RectShape::RectShape(QPoint startPoint,QPoint endPoint,QColor color,int penWidth)
    :Shape(startPoint,endPoint,color,penWidth){}
void RectShape::draw(QPainter& painter) const
{
    painter.setPen(QPen(color,penWidth));
    int x=std::min(startPoint.x(),endPoint.x());
    int y=std::min(startPoint.y(),endPoint.y());
    int width=std::abs(endPoint.x()-startPoint.x());
    int height=std::abs(endPoint.y()-startPoint.y());
    painter.drawRect(x,y,width,height);
}
QString RectShape::saveText() const
{
    return QString("Rect %1 %2 %3 %4 %5 %6 %7 %8")
        .arg(startPoint.x()).arg(startPoint.y())
        .arg(endPoint.x()).arg(endPoint.y())
        .arg(color.red()).arg(color.green()).arg(color.blue())
        .arg(penWidth);
}
bool RectShape::contains(const QPoint& point) const
{
    QPoint topLeft(std::min(startPoint.x(),endPoint.x()),std::min(startPoint.y(),endPoint.y()));
    QPoint topRight(std::max(startPoint.x(),endPoint.x()),std::min(startPoint.y(),endPoint.y()));
    QPoint bottomRight(std::max(startPoint.x(),endPoint.x()),std::max(startPoint.y(),endPoint.y()));
    QPoint bottomLeft(std::min(startPoint.x(),endPoint.x()),std::max(startPoint.y(),endPoint.y()));
    return nearSegment(point,topLeft,topRight,penWidth)
        || nearSegment(point,topRight,bottomRight,penWidth)
        || nearSegment(point,bottomRight,bottomLeft,penWidth)
        || nearSegment(point,bottomLeft,topLeft,penWidth);
}
//EllipseShape
EllipseShape::EllipseShape(QPoint startPoint,QPoint endPoint,QColor color,int penWidth)
    :Shape(startPoint,endPoint,color,penWidth){}
void EllipseShape::draw(QPainter& painter) const
{
    painter.setPen(QPen(color,penWidth));
    int x=std::min(startPoint.x(),endPoint.x());
    int y=std::min(startPoint.y(),endPoint.y());
    int width=std::abs(endPoint.x()-startPoint.x());
    int height=std::abs(endPoint.y()-startPoint.y());
    painter.drawEllipse(x,y,width,height);
}
QString EllipseShape::saveText() const
{
    return QString("Ellipse %1 %2 %3 %4 %5 %6 %7 %8")
        .arg(startPoint.x()).arg(startPoint.y())
        .arg(endPoint.x()).arg(endPoint.y())
        .arg(color.red()).arg(color.green()).arg(color.blue())
        .arg(penWidth);
}
bool EllipseShape::contains(const QPoint& point) const
{
    double width=std::abs(endPoint.x()-startPoint.x());
    double height=std::abs(endPoint.y()-startPoint.y());
    if(width==0 || height==0)
        return false;
    double rx=width/2.0;
    double ry=height/2.0;
    double cx=(startPoint.x()+endPoint.x())/2.0;
    double cy=(startPoint.y()+endPoint.y())/2.0;
    double value=std::pow((point.x()-cx)/rx,2)+std::pow((point.y()-cy)/ry,2);
    double tolerance=std::max(0.08,static_cast<double>(penWidth+4)/std::min(rx,ry));
    return std::abs(value-1.0)<=tolerance;
}
//DiamondShape
DiamondShape::DiamondShape(QPoint startPoint,QPoint endPoint,QColor color,int penWidth)
    :Shape(startPoint,endPoint,color,penWidth){}
void DiamondShape::draw(QPainter& painter) const
{
    painter.setPen(QPen(color,penWidth));
    int x1=std::min(startPoint.x(),endPoint.x());
    int x2=std::max(startPoint.x(),endPoint.x());
    int y1=std::min(startPoint.y(),endPoint.y());
    int y2=std::max(startPoint.y(),endPoint.y());
    int midX=(x1+x2)/2;
    int midY=(y1+y2)/2;
    QPoint top(midX,y1);
    QPoint right(x2,midY);
    QPoint bottom(midX,y2);
    QPoint left(x1,midY);
    QPolygon polygon;
    polygon << top << right << bottom << left;
    painter.drawPolygon(polygon);
}
QString DiamondShape::saveText() const
{
    return QString("Diamond %1 %2 %3 %4 %5 %6 %7 %8")
        .arg(startPoint.x()).arg(startPoint.y())
        .arg(endPoint.x()).arg(endPoint.y())
        .arg(color.red()).arg(color.green()).arg(color.blue())
        .arg(penWidth);
}
bool DiamondShape::contains(const QPoint& point) const
{
    int x1=std::min(startPoint.x(),endPoint.x());
    int x2=std::max(startPoint.x(),endPoint.x());
    int y1=std::min(startPoint.y(),endPoint.y());
    int y2=std::max(startPoint.y(),endPoint.y());
    int midX=(x1+x2)/2;
    int midY=(y1+y2)/2;
    QPoint top(midX,y1);
    QPoint right(x2,midY);
    QPoint bottom(midX,y2);
    QPoint left(x1,midY);
    return nearSegment(point,top,right,penWidth)
        || nearSegment(point,right,bottom,penWidth)
        || nearSegment(point,bottom,left,penWidth)
        || nearSegment(point,left,top,penWidth);
}
