#include"curve_shape.h"
#include"geometry_utils.h"
#include<QPen>
#include<QBrush>

CurveShape::CurveShape(CurveType curveType,const std::vector<QPointF>& controlPoints,const QColor& color,int penWidth,int degree)
    :Shape(color,penWidth),curveType(curveType),controlPoints(controlPoints),degree(degree){}

QPointF CurveShape::controlPointAt(int index) const
{
    if(index<0 || index>=static_cast<int>(controlPoints.size())) return QPointF();
    return controlPoints[index];
}

void CurveShape::moveControlPoint(int index,const QPointF& point)
{
    if(index<0 || index>=static_cast<int>(controlPoints.size())) return;
    controlPoints[index]=point;
}

void CurveShape::draw(QPainter& painter) const
{
    painter.save();
    painter.setPen(QPen(Qt::darkGray,1,Qt::DashLine));
    if(curveType==CurveType::Hermite && controlPoints.size()>=4)
    {
        painter.drawLine(controlPoints[0],controlPoints[1]);
        painter.drawLine(controlPoints[2],controlPoints[3]);
    }else
    {
        for(int i=1;i<static_cast<int>(controlPoints.size());++i)
            painter.drawLine(controlPoints[i-1],controlPoints[i]);
    }
    painter.setPen(QPen(Qt::black,1));
    painter.setBrush(QBrush(Qt::yellow));
    for(const QPointF& point:controlPoints)
        painter.drawEllipse(point,4,4);
    std::vector<QPointF> curve=generateCurvePoints();
    painter.setBrush(Qt::NoBrush);
    painter.setPen(QPen(color,penWidth));
    for(int i=1;i<static_cast<int>(curve.size());++i)
        painter.drawLine(curve[i-1],curve[i]);
    painter.restore();
}

QString CurveShape::typeName() const
{
    if(curveType==CurveType::Hermite) return "Hermite";
    if(curveType==CurveType::Bezier) return "Bezier";
    return "B-Spline";
}

std::vector<QPointF> CurveShape::generateCurvePoints() const
{
    std::vector<QPointF> points;
    const int steps=100;
    if(curveType==CurveType::Hermite)
    {
        if(controlPoints.size()<4) return points;
        for(int i=0;i<=steps;++i)
            points.push_back(generateHermitePoint(i/static_cast<double>(steps)));
    }else if(curveType==CurveType::Bezier)
    {
        if(static_cast<int>(controlPoints.size())<degree+1) return points;
        for(int i=0;i<=steps;++i)
            points.push_back(generateBezierPoint(i/static_cast<double>(steps)));
    }else
    {
        if(static_cast<int>(controlPoints.size())<degree+1) return points;
        int segmentCount=static_cast<int>(controlPoints.size())-degree;
        for(int segment=0;segment<segmentCount;++segment)
            for(int i=0;i<=steps;++i)
                points.push_back(generateBSplinePoint(segment,i/static_cast<double>(steps)));
    }
    return points;
}

QPointF CurveShape::generateHermitePoint(double t) const
{
    QPointF p0=controlPoints[0];
    QPointF p1=controlPoints[2];
    QPointF t0=controlPoints[1]-p0;
    QPointF t1=controlPoints[3]-p1;
    double t2=t*t;
    double t3=t2*t;
    double h00=2*t3-3*t2+1;
    double h01=-2*t3+3*t2;
    double h10=t3-2*t2+t;
    double h11=t3-t2;
    return p0*h00+p1*h01+t0*h10+t1*h11;
}

QPointF CurveShape::generateBezierPoint(double t) const
{
    std::vector<QPointF> temp;
    for(int i=0;i<=degree && i<static_cast<int>(controlPoints.size());++i)
        temp.push_back(controlPoints[i]);
    for(int level=static_cast<int>(temp.size())-1;level>0;--level)
        for(int i=0;i<level;++i)
            temp[i]=lerpPoint(temp[i],temp[i+1],t);
    return temp.front();
}

QPointF CurveShape::generateBSplinePoint(int segmentIndex,double t) const
{
    QPointF p0=controlPoints[segmentIndex];
    QPointF p1=controlPoints[segmentIndex+1];
    QPointF p2=controlPoints[segmentIndex+2];
    if(degree==2)
    {
        double t2=t*t;
        double b0=(1-2*t+t2)/2.0;
        double b1=(-2*t2+2*t+1)/2.0;
        double b2=t2/2.0;
        return p0*b0+p1*b1+p2*b2;
    }
    QPointF p3=controlPoints[segmentIndex+3];
    double t2=t*t;
    double t3=t2*t;
    double b0=(-t3+3*t2-3*t+1)/6.0;
    double b1=(3*t3-6*t2+4)/6.0;
    double b2=(-3*t3+3*t2+3*t+1)/6.0;
    double b3=t3/6.0;
    return p0*b0+p1*b1+p2*b2+p3*b3;
}
