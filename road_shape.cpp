#include"road_shape.h"
#include"geometry_utils.h"
#include<QPen>
#include<QBrush>

RoadShape::RoadShape(const std::vector<QPointF>& centerPoints,double roadWidth,const QColor& color,int penWidth)
    :Shape(color,penWidth),centerPoints(centerPoints),roadWidth(roadWidth){}

QPointF RoadShape::controlPointAt(int index) const
{
    if(index<0 || index>=static_cast<int>(centerPoints.size())) return QPointF();
    return centerPoints[index];
}

void RoadShape::moveControlPoint(int index,const QPointF& point)
{
    if(index<0 || index>=static_cast<int>(centerPoints.size())) return;
    centerPoints[index]=point;
}

void RoadShape::draw(QPainter& painter) const
{
    if(centerPoints.size()<2) return;
    std::vector<QPointF> left=buildLeftBoundary();
    std::vector<QPointF> right=buildRightBoundary();
    painter.save();
    painter.setBrush(Qt::NoBrush);
    painter.setPen(QPen(color,penWidth));
    for(int i=1;i<static_cast<int>(left.size());++i)
        painter.drawLine(left[i-1],left[i]);
    for(int i=1;i<static_cast<int>(right.size());++i)
        painter.drawLine(right[i-1],right[i]);
    painter.setPen(QPen(Qt::blue,1,Qt::DashLine));
    for(int i=1;i<static_cast<int>(centerPoints.size());++i)
        painter.drawLine(centerPoints[i-1],centerPoints[i]);
    painter.setPen(QPen(Qt::black,1));
    painter.setBrush(QBrush(Qt::yellow));
    for(const QPointF& point:centerPoints)
        painter.drawEllipse(point,4,4);
    painter.restore();
}

std::vector<QPointF> RoadShape::buildLeftBoundary() const
{
    return buildBoundary(1.0);
}

std::vector<QPointF> RoadShape::buildRightBoundary() const
{
    return buildBoundary(-1.0);
}

QPolygonF RoadShape::buildRoadPolygon() const
{
    std::vector<QPointF> left=buildLeftBoundary();
    std::vector<QPointF> right=buildRightBoundary();
    QPolygonF polygon;
    for(const QPointF& point:left)
        polygon << point;
    for(auto it=right.rbegin();it!=right.rend();++it)
        polygon << *it;
    return polygon;
}

std::vector<QPointF> RoadShape::buildBoundary(double side) const
{
    std::vector<QPointF> boundary;
    int n=static_cast<int>(centerPoints.size());
    if(n<2) return boundary;
    double halfWidth=roadWidth/2.0;
    std::vector<QPointF> offsets;
    for(int i=0;i<n-1;++i)
    {
        QPointF direction=centerPoints[i+1]-centerPoints[i];
        QPointF unit=normalizeVector(direction);
        offsets.push_back(QPointF(-unit.y()*side,unit.x()*side)*halfWidth);
    }
    boundary.push_back(centerPoints.front()+offsets.front());
    for(int i=1;i<n-1;++i)
    {
        QPointF previousDirection=centerPoints[i]-centerPoints[i-1];
        QPointF nextDirection=centerPoints[i+1]-centerPoints[i];
        QPointF previousOffsetPoint=centerPoints[i]+offsets[i-1];
        QPointF nextOffsetPoint=centerPoints[i]+offsets[i];
        QPointF a2=previousOffsetPoint+previousDirection;
        QPointF b2=nextOffsetPoint+nextDirection;
        QPointF cross;
        if(lineIntersection(previousOffsetPoint,a2,nextOffsetPoint,b2,cross))
            boundary.push_back(cross);
        else
            boundary.push_back((previousOffsetPoint+nextOffsetPoint)*0.5);
    }
    boundary.push_back(centerPoints.back()+offsets.back());
    return boundary;
}
