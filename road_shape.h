#ifndef ROAD_SHAPE_H
#define ROAD_SHAPE_H
#include"shape.h"
#include<QPolygonF>
#include<QPointF>
#include<vector>

class RoadShape:public Shape
{
public:
    RoadShape(const std::vector<QPointF>& centerPoints,double roadWidth,const QColor& color,int penWidth);
    void draw(QPainter& painter) const override;
    QString typeName() const override{return "Road";}
    int controlPointCount() const override{return static_cast<int>(centerPoints.size());}
    QPointF controlPointAt(int index) const override;
    void moveControlPoint(int index,const QPointF& point) override;
    std::vector<QPointF> buildLeftBoundary() const;
    std::vector<QPointF> buildRightBoundary() const;
    QPolygonF buildRoadPolygon() const;

private:
    std::vector<QPointF> buildBoundary(double side) const;

    std::vector<QPointF> centerPoints;
    double roadWidth;
};

#endif //ROAD_SHAPE_H
