#ifndef SHAPE_SAVED_H
#define SHAPE_SAVED_H
#include"shape.h"
#include<memory>
#include<vector>
class ShapeSaved
{
public:
    ShapeSaved();
    ~ShapeSaved();
    void addShape(std::unique_ptr<Shape> shape);
    void removeShapeAt(int index);
    void clearAllShapes();
    const std::vector<std::unique_ptr<Shape>>& getShapes() const {return shapes;}
    std::vector<std::unique_ptr<Shape>>& getShapes() {return shapes;}
    int getShapeCount() const {return shapes.size();}
private:
    std::vector<std::unique_ptr<Shape>> shapes;
};
#endif //SHAPE_SAVED_H
