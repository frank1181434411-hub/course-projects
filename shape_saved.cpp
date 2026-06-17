#include"shape_saved.h"
ShapeSaved::ShapeSaved(){}
ShapeSaved::~ShapeSaved(){}
void ShapeSaved::addShape(std::unique_ptr<Shape> shape) {shapes.push_back(std::move(shape));}
void ShapeSaved::removeShapeAt(int index)
{
    if(index < 0 || index >= static_cast<int>(shapes.size())) return;
    shapes.erase(shapes.begin()+index);
}
void ShapeSaved::clearAllShapes(){shapes.clear();}
