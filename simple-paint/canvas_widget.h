#ifndef CANVAS_WIDGET_H
#define CANVAS_WIDGET_H
#include<QWidget>
#include<QPoint>
#include<QColor>
#include<memory>
#include<vector>
class Shape;
class ShapeSaved;
enum class DrawMode
{
    Line,Rectangle,Ellipse,Diamond,Select,None
};
class CanvasWidget:public QWidget
{
    Q_OBJECT
public:
    CanvasWidget(QWidget* parent=nullptr);
    ~CanvasWidget();
    void setDrawMode(DrawMode mode) {currentMode=mode;}
    void setPenColor(QColor color) {penColor=color;}
    void setPenWidth(int width) {penWidth=width;}
    ShapeSaved* getShapeSaved() {return shapeSaved.get();}
    void clearCanvas();
    void deleteSelectedShape();
    void clearSelection() {selectedShapeIndexes.clear(); update();}
protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void paintEvent(QPaintEvent* event) override;
private:
    std::unique_ptr<ShapeSaved> shapeSaved;
    DrawMode currentMode=DrawMode::None;
    QColor penColor=Qt::black;
    int penWidth=1;
    bool isDrawing=false;
    QPoint startPoint;
    QPoint currentEndPoint;
    std::unique_ptr<Shape> previewShape;
    std::vector<int> selectedShapeIndexes;
    void createPreviewShape();
    void finalizeShape();
    void selectShapeAt(const QPoint& point,Qt::KeyboardModifiers modifiers);
    void drawSelection(QPainter& painter,const Shape* shape);
    QPoint applyShiftConstraint(const QPoint& point,Qt::KeyboardModifiers modifiers) const;
    bool isShapeSelected(int index) const;
};
#endif //CANVAS_WIDGET_H
