#ifndef CANVAS_WIDGET_H
#define CANVAS_WIDGET_H
#include<QWidget>
#include<QPointF>
#include<QColor>
#include<QString>
#include<memory>
#include<vector>

class ShapeSaved;
class PolygonShape;
class Shape;

enum class DrawMode
{
    Road,
    Polygon,
    PointTest,
    Hermite,
    Bezier,
    BSpline,
    EditPoints,
    None
};

class CanvasWidget:public QWidget
{
    Q_OBJECT
public:
    CanvasWidget(QWidget* parent=nullptr);
    ~CanvasWidget();
    void setDrawMode(DrawMode mode);
    void setPenColor(QColor color) {penColor=color;}
    void setPenWidth(int width) {penWidth=width;}
    void setRoadWidth(double width) {roadWidth=width;}
    void setBezierDegree(int degree) {bezierDegree=degree;}
    void setBSplineDegree(int degree) {bSplineDegree=degree;}
    ShapeSaved* getShapeSaved() {return shapeSaved.get();}
    void clearCanvas();

signals:
    void statusMessageChanged(const QString& message);

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void paintEvent(QPaintEvent* event) override;

private:
    std::unique_ptr<ShapeSaved> shapeSaved;
    DrawMode currentMode=DrawMode::None;
    QColor penColor=Qt::black;
    int penWidth=2;
    double roadWidth=40.0;
    int bezierDegree=3;
    int bSplineDegree=3;
    std::vector<QPointF> currentPoints;
    QPointF hoverPoint;
    QPointF lastTestPoint;
    QString lastTestResult;
    bool hasHoverPoint=false;
    bool hasLastTestPoint=false;
    Shape* draggedShape=nullptr;
    int draggedControlPointIndex=-1;
    bool isDraggingControlPoint=false;

    void addPoint(const QPointF& point);
    void finishInput();
    void beginMoveControlPoint(const QPointF& point);
    void moveActiveControlPoint(const QPointF& point);
    void endMoveControlPoint();
    PolygonShape* findLastPolygon() const;
    void drawPreview(QPainter& painter) const;
    void drawCurrentPolyline(QPainter& painter,bool closed) const;
    void drawCurrentPoints(QPainter& painter) const;
    void testPointRelation(const QPointF& point);
};

#endif //CANVAS_WIDGET_H
