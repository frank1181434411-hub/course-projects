#ifndef GEOMETRY_UTILS_H
#define GEOMETRY_UTILS_H
#include<QPointF>
#include<vector>

enum class PointRelation
{
    Inside,
    Outside,
    Boundary
};

double vectorLength(const QPointF& vector);
QPointF normalizeVector(const QPointF& vector);
QPointF lerpPoint(const QPointF& a,const QPointF& b,double t);
double distancePointToSegment(const QPointF& point,const QPointF& a,const QPointF& b);
bool isPointOnSegment(const QPointF& point,const QPointF& a,const QPointF& b,double eps=1e-6);
bool lineIntersection(const QPointF& a1,const QPointF& a2,const QPointF& b1,const QPointF& b2,QPointF& result,double eps=1e-9);
PointRelation classifyPointInPolygon(const QPointF& point,const std::vector<QPointF>& polygon,double eps=1e-6);

#endif //GEOMETRY_UTILS_H
