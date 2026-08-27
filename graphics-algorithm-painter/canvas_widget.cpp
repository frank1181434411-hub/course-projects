#include"canvas_widget.h"
#include"shape.h"
#include"shape_saved.h"
#include"road_shape.h"
#include"polygon_shape.h"
#include"curve_shape.h"
#include<QMouseEvent>
#include<QPainter>
#include<QPen>
#include<QBrush>

CanvasWidget::CanvasWidget(QWidget* parent)
    :QWidget(parent),shapeSaved(std::make_unique<ShapeSaved>())
{
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    setStyleSheet("background-color: white;");
}

CanvasWidget::~CanvasWidget(){}

void CanvasWidget::setDrawMode(DrawMode mode)
{
    currentMode=mode;
    currentPoints.clear();
    hasHoverPoint=false;
    endMoveControlPoint();
    update();
}

void CanvasWidget::mousePressEvent(QMouseEvent* event)
{
    if(currentMode==DrawMode::None) return;
    if(currentMode==DrawMode::EditPoints)
    {
        if(event->button()==Qt::LeftButton)
            beginMoveControlPoint(event->position());
        update();
        return;
    }
    if(event->button()==Qt::RightButton)
    {
        finishInput();
        update();
        return;
    }
    if(event->button()!=Qt::LeftButton) return;
    if(currentMode==DrawMode::PointTest)
        testPointRelation(event->position());
    else
        addPoint(event->position());
    update();
}

void CanvasWidget::mouseMoveEvent(QMouseEvent* event)
{
    hoverPoint=event->position();
    hasHoverPoint=true;
    if(currentMode==DrawMode::EditPoints && isDraggingControlPoint)
        moveActiveControlPoint(event->position());
    update();
}

void CanvasWidget::mouseReleaseEvent(QMouseEvent*)
{
    if(currentMode==DrawMode::EditPoints)
        endMoveControlPoint();
}

void CanvasWidget::mouseDoubleClickEvent(QMouseEvent* event)
{
    if(currentMode==DrawMode::EditPoints) return;
    if(currentMode==DrawMode::PointTest && event->button()==Qt::LeftButton)
        testPointRelation(event->position());
    else
        finishInput();
    update();
}

void CanvasWidget::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing,true);
    painter.fillRect(rect(),Qt::white);
    const auto& shapes=shapeSaved->getShapes();
    for(const auto& shape:shapes)
        shape->draw(painter);
    drawPreview(painter);
}

void CanvasWidget::addPoint(const QPointF& point)
{
    currentPoints.push_back(point);
    if(currentMode==DrawMode::Hermite && currentPoints.size()==4)
    {
        shapeSaved->addShape(std::make_unique<CurveShape>(CurveType::Hermite,currentPoints,penColor,penWidth));
        currentPoints.clear();
        emit statusMessageChanged("Hermite curve generated");
    }else if(currentMode==DrawMode::Bezier && static_cast<int>(currentPoints.size())==bezierDegree+1)
    {
        shapeSaved->addShape(std::make_unique<CurveShape>(CurveType::Bezier,currentPoints,penColor,penWidth,bezierDegree));
        currentPoints.clear();
        emit statusMessageChanged(QString("Bezier degree %1 curve generated").arg(bezierDegree));
    }
}

void CanvasWidget::finishInput()
{
    if(currentMode==DrawMode::Road && currentPoints.size()>=2)
    {
        shapeSaved->addShape(std::make_unique<RoadShape>(currentPoints,roadWidth,penColor,penWidth));
        currentPoints.clear();
        emit statusMessageChanged("Road generated");
    }else if(currentMode==DrawMode::Polygon && currentPoints.size()>=3)
    {
        shapeSaved->addShape(std::make_unique<PolygonShape>(currentPoints,penColor,penWidth));
        currentPoints.clear();
        emit statusMessageChanged("Polygon generated");
    }else if(currentMode==DrawMode::Bezier && static_cast<int>(currentPoints.size())>=bezierDegree+1)
    {
        shapeSaved->addShape(std::make_unique<CurveShape>(CurveType::Bezier,currentPoints,penColor,penWidth,bezierDegree));
        currentPoints.clear();
        emit statusMessageChanged(QString("Bezier degree %1 curve generated").arg(bezierDegree));
    }else if(currentMode==DrawMode::BSpline && static_cast<int>(currentPoints.size())>=bSplineDegree+1)
    {
        shapeSaved->addShape(std::make_unique<CurveShape>(CurveType::BSpline,currentPoints,penColor,penWidth,bSplineDegree));
        currentPoints.clear();
        emit statusMessageChanged(QString("B-Spline degree %1 curve generated").arg(bSplineDegree));
    }
}

void CanvasWidget::clearCanvas()
{
    shapeSaved->clearAllShapes();
    currentPoints.clear();
    lastTestResult.clear();
    hasLastTestPoint=false;
    endMoveControlPoint();
    update();
}

void CanvasWidget::beginMoveControlPoint(const QPointF& point)
{
    const double hitRadius=8.0;
    auto& shapes=shapeSaved->getShapes();
    for(auto it=shapes.rbegin();it!=shapes.rend();++it)
    {
        int index=(*it)->hitControlPoint(point,hitRadius);
        if(index>=0)
        {
            draggedShape=it->get();
            draggedControlPointIndex=index;
            isDraggingControlPoint=true;
            moveActiveControlPoint(point);
            emit statusMessageChanged("Edit points: "+draggedShape->typeName());
            return;
        }
    }
    emit statusMessageChanged("Edit points: no control point selected");
}

void CanvasWidget::moveActiveControlPoint(const QPointF& point)
{
    if(!isDraggingControlPoint || !draggedShape || draggedControlPointIndex<0) return;
    draggedShape->moveControlPoint(draggedControlPointIndex,point);
}

void CanvasWidget::endMoveControlPoint()
{
    draggedShape=nullptr;
    draggedControlPointIndex=-1;
    isDraggingControlPoint=false;
}

PolygonShape* CanvasWidget::findLastPolygon() const
{
    auto& shapes=shapeSaved->getShapes();
    for(auto it=shapes.rbegin();it!=shapes.rend();++it)
    {
        PolygonShape* polygon=dynamic_cast<PolygonShape*>(it->get());
        if(polygon) return polygon;
    }
    return nullptr;
}

void CanvasWidget::drawPreview(QPainter& painter) const
{
    bool closed=currentMode==DrawMode::Polygon;
    if(currentMode==DrawMode::Road || currentMode==DrawMode::Polygon || currentMode==DrawMode::Bezier || currentMode==DrawMode::BSpline || currentMode==DrawMode::Hermite)
    {
        drawCurrentPolyline(painter,closed);
        drawCurrentPoints(painter);
    }
    if(hasLastTestPoint)
    {
        painter.save();
        painter.setPen(QPen(Qt::darkRed,2));
        painter.setBrush(QBrush(Qt::red));
        painter.drawEllipse(lastTestPoint,4,4);
        painter.drawText(lastTestPoint+QPointF(8,-8),lastTestResult);
        painter.restore();
    }
}

void CanvasWidget::drawCurrentPolyline(QPainter& painter,bool closed) const
{
    if(currentPoints.empty()) return;
    painter.save();
    painter.setPen(QPen(Qt::darkGray,1,Qt::DashLine));
    for(int i=1;i<static_cast<int>(currentPoints.size());++i)
        painter.drawLine(currentPoints[i-1],currentPoints[i]);
    if(hasHoverPoint)
        painter.drawLine(currentPoints.back(),hoverPoint);
    if(closed && currentPoints.size()>=3)
        painter.drawLine(currentPoints.back(),currentPoints.front());
    painter.restore();
}

void CanvasWidget::drawCurrentPoints(QPainter& painter) const
{
    painter.save();
    painter.setPen(QPen(Qt::black,1));
    painter.setBrush(QBrush(Qt::yellow));
    for(const QPointF& point:currentPoints)
        painter.drawEllipse(point,4,4);
    painter.restore();
}

void CanvasWidget::testPointRelation(const QPointF& point)
{
    lastTestPoint=point;
    hasLastTestPoint=true;
    PolygonShape* polygon=findLastPolygon();
    if(!polygon)
    {
        lastTestResult="No polygon";
        emit statusMessageChanged("Point relation: No polygon");
        return;
    }
    PointRelation relation=polygon->classifyPoint(point);
    if(relation==PointRelation::Inside) lastTestResult="Inside";
    else if(relation==PointRelation::Outside) lastTestResult="Outside";
    else lastTestResult="Boundary";
    emit statusMessageChanged("Point relation: "+lastTestResult);
}
