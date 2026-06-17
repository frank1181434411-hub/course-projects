#ifndef EXSHAPE_H
#define EXSHAPE_H
#include"shape.h"
class LineShape : public Shape
{
public:
    LineShape(QPoint startPoint, QPoint endPoint, QColor color, int penWidth);
    void draw(QPainter& painter) const override;
    QString typeName() const override { return "Line"; }
    QString saveText() const override;
    bool contains(const QPoint& point) const override;
};
class RectShape : public Shape
{
public:
    RectShape(QPoint startPoint, QPoint endPoint, QColor color, int penWidth);
    void draw(QPainter& painter) const override;
    QString typeName() const override { return "Rect"; }
    QString saveText() const override;
    bool contains(const QPoint& point) const override;
};
class EllipseShape : public Shape
{
public:
    EllipseShape(QPoint startPoint, QPoint endPoint, QColor color, int penWidth);
    void draw(QPainter& painter) const override;
    QString typeName() const override { return "Ellipse"; }
    QString saveText() const override;
    bool contains(const QPoint& point) const override;
};
class DiamondShape : public Shape
{
public:
    DiamondShape(QPoint startPoint, QPoint endPoint, QColor color, int penWidth);
    void draw(QPainter& painter) const override;
    QString typeName() const override { return "Diamond"; }
    QString saveText() const override;
    bool contains(const QPoint& point) const override;
};
#endif //EXSHAPE_H
