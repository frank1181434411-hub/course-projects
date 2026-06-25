#ifndef POLYGON_SHAPE_H
#define POLYGON_SHAPE_H
#include"shape.h"
#include"geometry_utils.h"
#include<QPointF>
#include<vector>

class PolygonShape:public Shape
{
public:
    PolygonShape(const std::vector<QPointF>& vertices,const QColor& color,int penWidth);
    void draw(QPainter& painter) const override;
    QString typeName() const override{return "Polygon";}
    int controlPointCount() const override{return static_cast<int>(vertices.size());}
    QPointF controlPointAt(int index) const override;
    void moveControlPoint(int index,const QPointF& point) override;
    PointRelation classifyPoint(const QPointF& point) const;
    const std::vector<QPointF>& getVertices() const{return vertices;}

private:
    std::vector<QPointF> vertices;
};

#endif //POLYGON_SHAPE_H
