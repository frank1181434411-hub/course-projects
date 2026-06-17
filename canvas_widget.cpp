#include"canvas_widget.h"
#include"shape.h"
#include"exshape.h"
#include"shape_saved.h"
#include<QMouseEvent>
#include<QPainter>
#include<QPen>
#include<QRect>
#include<algorithm>
#include<cmath>
#include<functional>
CanvasWidget::CanvasWidget(QWidget* parent)
    :QWidget(parent),shapeSaved(std::make_unique<ShapeSaved>())
{
    setFocusPolicy(Qt::StrongFocus);
    setStyleSheet("background-color: white;");
}
CanvasWidget::~CanvasWidget() {}
void CanvasWidget::mousePressEvent(QMouseEvent* event)
{
    if(currentMode==DrawMode::Select)
    {
        selectShapeAt(event->pos(),event->modifiers());
        update();
        return;
    }
    if(currentMode==DrawMode::None) return;
    isDrawing=true;
    startPoint=event->pos();
    currentEndPoint=event->pos();
    previewShape=nullptr;
}
void CanvasWidget::mouseMoveEvent(QMouseEvent* event)
{
    if(!isDrawing||currentMode==DrawMode::None) return;
    currentEndPoint=applyShiftConstraint(event->pos(),event->modifiers());
    createPreviewShape();
    update();
}
void CanvasWidget::mouseReleaseEvent(QMouseEvent* event)
{
    if(!isDrawing||currentMode==DrawMode::None)
    {
        isDrawing=false;
        return;
    }
    currentEndPoint=applyShiftConstraint(event->pos(),event->modifiers());
    finalizeShape();
    previewShape=nullptr;
    isDrawing=false;
    update();
}
void CanvasWidget::paintEvent(QPaintEvent* /*event*/)
{
    QPainter painter(this);
    painter.fillRect(rect(),Qt::white);
    //Draw all saved shapes
    const auto& shapes=shapeSaved->getShapes();
    for(int i=0;i<static_cast<int>(shapes.size());i++)
    {
        shapes[i]->draw(painter);
        if(isShapeSelected(i))
            drawSelection(painter,shapes[i].get());
    }
    //Draw preview shape if available
    if(previewShape)
        previewShape->draw(painter);
}
void CanvasWidget::createPreviewShape()
{
    previewShape=nullptr;
    switch(currentMode)
    {
        case DrawMode::Line:
            previewShape=std::make_unique<LineShape>(startPoint,currentEndPoint,penColor,penWidth);
            break;
        case DrawMode::Rectangle:
            previewShape=std::make_unique<RectShape>(startPoint,currentEndPoint,penColor,penWidth);
            break;
        case DrawMode::Ellipse:
            previewShape=std::make_unique<EllipseShape>(startPoint,currentEndPoint,penColor,penWidth);
            break;
        case DrawMode::Diamond:
            previewShape=std::make_unique<DiamondShape>(startPoint,currentEndPoint,penColor,penWidth);
            break;
        default:
            break;
    }
}
void CanvasWidget::finalizeShape()
{
    createPreviewShape();
    if(previewShape)
    {
        shapeSaved->addShape(std::move(previewShape));
        selectedShapeIndexes.clear();
    }
}
void CanvasWidget::clearCanvas()
{
    shapeSaved->clearAllShapes();
    selectedShapeIndexes.clear();
    update();
}
void CanvasWidget::deleteSelectedShape()
{
    if(selectedShapeIndexes.empty()) return;
    std::sort(selectedShapeIndexes.begin(),selectedShapeIndexes.end(),std::greater<int>());
    selectedShapeIndexes.erase(std::unique(selectedShapeIndexes.begin(),selectedShapeIndexes.end()),selectedShapeIndexes.end());
    for(int index:selectedShapeIndexes)
        shapeSaved->removeShapeAt(index);
    selectedShapeIndexes.clear();
    update();
}
void CanvasWidget::selectShapeAt(const QPoint& point,Qt::KeyboardModifiers modifiers)
{
    const auto& shapes=shapeSaved->getShapes();
    for(int i=static_cast<int>(shapes.size())-1;i>=0;i--)
    {
        if(shapes[i]->contains(point))
        {
            auto it=std::find(selectedShapeIndexes.begin(),selectedShapeIndexes.end(),i);
            if(modifiers&Qt::ControlModifier)
            {
                if(it==selectedShapeIndexes.end())
                    selectedShapeIndexes.push_back(i);
                else
                    selectedShapeIndexes.erase(it);
            }else
            {
                selectedShapeIndexes.clear();
                selectedShapeIndexes.push_back(i);
            }
            return;
        }
    }
    if(!(modifiers&Qt::ControlModifier))
        selectedShapeIndexes.clear();
}
void CanvasWidget::drawSelection(QPainter& painter,const Shape* shape)
{
    int x=std::min(shape->getStartPoint().x(),shape->getEndPoint().x());
    int y=std::min(shape->getStartPoint().y(),shape->getEndPoint().y());
    int width=std::abs(shape->getEndPoint().x()-shape->getStartPoint().x());
    int height=std::abs(shape->getEndPoint().y()-shape->getStartPoint().y());
    QRect rect(x,y,width,height);
    rect=rect.adjusted(-6,-6,6,6);
    painter.save();
    painter.setPen(QPen(Qt::red,1,Qt::DashLine));
    painter.drawRect(rect);
    painter.restore();
}
QPoint CanvasWidget::applyShiftConstraint(const QPoint& point,Qt::KeyboardModifiers modifiers) const
{
    if(!(modifiers&Qt::ShiftModifier)) return point;
    if(currentMode!=DrawMode::Rectangle&&currentMode!=DrawMode::Ellipse&&currentMode!=DrawMode::Diamond) return point;
    int dx=point.x()-startPoint.x();
    int dy=point.y()-startPoint.y();
    int size=std::min(std::abs(dx),std::abs(dy));
    int x=startPoint.x()+(dx<0?-size:size);
    int y=startPoint.y()+(dy<0?-size:size);
    return QPoint(x,y);
}
bool CanvasWidget::isShapeSelected(int index) const
{
    return std::find(selectedShapeIndexes.begin(),selectedShapeIndexes.end(),index)!=selectedShapeIndexes.end();
}
