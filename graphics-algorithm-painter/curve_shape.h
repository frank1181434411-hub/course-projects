#ifndef CURVE_SHAPE_H
#define CURVE_SHAPE_H
#include"shape.h"
#include<QPointF>
#include<vector>

enum class CurveType
{
    Hermite,
    Bezier,
    BSpline
};

class CurveShape:public Shape
{
public:
    CurveShape(CurveType curveType,const std::vector<QPointF>& controlPoints,const QColor& color,int penWidth,int degree=3);
    void draw(QPainter& painter) const override;
    QString typeName() const override;
    int controlPointCount() const override{return static_cast<int>(controlPoints.size());}
    QPointF controlPointAt(int index) const override;
    void moveControlPoint(int index,const QPointF& point) override;
    std::vector<QPointF> generateCurvePoints() const;
    QPointF generateHermitePoint(double t) const;
    QPointF generateBezierPoint(double t) const;
    QPointF generateBSplinePoint(int segmentIndex,double t) const;

private:
    CurveType curveType;
    std::vector<QPointF> controlPoints;
    int degree;
};

#endif //CURVE_SHAPE_H
