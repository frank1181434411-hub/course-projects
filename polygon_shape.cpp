#include"polygon_shape.h"
#include<QPolygonF>
#include<QPen>
#include<QBrush>

PolygonShape::PolygonShape(const std::vector<QPointF>& vertices,const QColor& color,int penWidth)
    :Shape(color,penWidth),vertices(vertices){}

QPointF PolygonShape::controlPointAt(int index) const
{
    if(index<0 || index>=static_cast<int>(vertices.size())) return QPointF();
    return vertices[index];
}

void PolygonShape::moveControlPoint(int index,const QPointF& point)
{
    if(index<0 || index>=static_cast<int>(vertices.size())) return;
    vertices[index]=point;
}

void PolygonShape::draw(QPainter& painter) const
{
    if(vertices.empty()) return;
    QPolygonF polygon;
    for(const QPointF& point:vertices)
        polygon << point;
    painter.save();
    painter.setPen(QPen(color,penWidth));
    painter.setBrush(QBrush(QColor(180,220,255,80)));
    if(vertices.size()>=3)
        painter.drawPolygon(polygon);
    else
        painter.drawPolyline(polygon);
    painter.setPen(QPen(Qt::black,1));
    painter.setBrush(QBrush(Qt::yellow));
    for(const QPointF& point:vertices)
        painter.drawEllipse(point,4,4);
    painter.restore();
}

PointRelation PolygonShape::classifyPoint(const QPointF& point) const
{
    return classifyPointInPolygon(point,vertices,3.0);
}
