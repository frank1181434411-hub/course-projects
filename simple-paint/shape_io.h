#ifndef SHAPE_IO_H
#define SHAPE_IO_H
#include<QString>
#include<memory>
class Shape;
class ShapeSaved;
class ShapeIO
{
public:
    static bool saveToFile(const ShapeSaved& shapeSaved,const QString& filePath);
    static bool loadFromFile(ShapeSaved& shapeSaved,const QString& filePath);
private:
    static std::unique_ptr<Shape> parseShapeLine(const QString& line);
};
#endif //SHAPE_IO_H
