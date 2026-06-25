#include"geometry_utils.h"
#include<cmath>
#include<algorithm>

static double crossProduct(const QPointF& a,const QPointF& b)
{
    return a.x()*b.y()-a.y()*b.x();
}

double vectorLength(const QPointF& vector)
{
    return std::sqrt(vector.x()*vector.x()+vector.y()*vector.y());
}

QPointF normalizeVector(const QPointF& vector)
{
    double len=vectorLength(vector);
    if(len<1e-9) return QPointF(0,0);
    return QPointF(vector.x()/len,vector.y()/len);
}

QPointF lerpPoint(const QPointF& a,const QPointF& b,double t)
{
    return QPointF(a.x()+(b.x()-a.x())*t,a.y()+(b.y()-a.y())*t);
}

double distancePointToSegment(const QPointF& point,const QPointF& a,const QPointF& b)
{
    QPointF ab=b-a;
    double len2=ab.x()*ab.x()+ab.y()*ab.y();
    if(len2<1e-9) return vectorLength(point-a);
    double t=((point.x()-a.x())*ab.x()+(point.y()-a.y())*ab.y())/len2;
    t=std::max(0.0,std::min(1.0,t));
    return vectorLength(point-lerpPoint(a,b,t));
}

bool isPointOnSegment(const QPointF& point,const QPointF& a,const QPointF& b,double eps)
{
    if(distancePointToSegment(point,a,b)>eps) return false;
    double minX=std::min(a.x(),b.x())-eps;
    double maxX=std::max(a.x(),b.x())+eps;
    double minY=std::min(a.y(),b.y())-eps;
    double maxY=std::max(a.y(),b.y())+eps;
    return point.x()>=minX && point.x()<=maxX && point.y()>=minY && point.y()<=maxY;
}

bool lineIntersection(const QPointF& a1,const QPointF& a2,const QPointF& b1,const QPointF& b2,QPointF& result,double eps)
{
    QPointF r=a2-a1;
    QPointF s=b2-b1;
    double denom=crossProduct(r,s);
    if(std::abs(denom)<eps) return false;
    double t=crossProduct(b1-a1,s)/denom;
    result=a1+r*t;
    return true;
}

PointRelation classifyPointInPolygon(const QPointF& point,const std::vector<QPointF>& polygon,double eps)
{
    if(polygon.size()<3) return PointRelation::Outside;
    bool inside=false;
    int n=static_cast<int>(polygon.size());
    for(int i=0,j=n-1;i<n;j=i++)
    {
        const QPointF& a=polygon[j];
        const QPointF& b=polygon[i];
        if(isPointOnSegment(point,a,b,eps)) return PointRelation::Boundary;
        bool crosses=(a.y()>point.y())!=(b.y()>point.y());
        if(crosses)
        {
            double x=(b.x()-a.x())*(point.y()-a.y())/(b.y()-a.y())+a.x();
            if(x>point.x()) inside=!inside;
        }
    }
    return inside?PointRelation::Inside:PointRelation::Outside;
}
