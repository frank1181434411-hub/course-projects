#include"shape_io.h"
#include"shape.h"
#include"exshape.h"
#include"shape_saved.h"
#include<QFile>
#include<QTextStream>
#include<QStringList>
bool ShapeIO::saveToFile(const ShapeSaved& shapeSaved,const QString& filePath)
{
    QFile file(filePath);
    if(!file.open(QIODevice::WriteOnly|QIODevice::Text))
        return false;
    QTextStream out(&file);
    for(const auto& shape:shapeSaved.getShapes())
        out << shape->saveText() << "\n";
    file.close();
    return true;
}
bool ShapeIO::loadFromFile(ShapeSaved& shapeSaved,const QString& filePath)
{
    QFile file(filePath);
    if(!file.open(QIODevice::ReadOnly|QIODevice::Text))
        return false;
    shapeSaved.clearAllShapes();
    QTextStream in(&file);
    while(!in.atEnd())
    {
        QString line=in.readLine().trimmed();
        if(line.isEmpty()) continue;
        auto shape=parseShapeLine(line);
        if(shape)
            shapeSaved.addShape(std::move(shape));
    }
    file.close();
    return true;
}
std::unique_ptr<Shape> ShapeIO::parseShapeLine(const QString& line)
{
    QStringList parts=line.split(" ");
    if(parts.size()<9)
        return nullptr;
    QString type=parts[0];
    int x1=parts[1].toInt();
    int y1=parts[2].toInt();
    int x2=parts[3].toInt();
    int y2=parts[4].toInt();
    int r=parts[5].toInt();
    int g=parts[6].toInt();
    int b=parts[7].toInt();
    int penWidth=parts[8].toInt();
    QPoint startPoint(x1,y1);
    QPoint endPoint(x2,y2);
    QColor color(r,g,b);
    if(type=="Line")
        return std::make_unique<LineShape>(startPoint,endPoint,color,penWidth);
    else if(type=="Rect")
        return std::make_unique<RectShape>(startPoint,endPoint,color,penWidth);
    else if(type=="Ellipse")
        return std::make_unique<EllipseShape>(startPoint,endPoint,color,penWidth);
    else if(type=="Diamond")
        return std::make_unique<DiamondShape>(startPoint,endPoint,color,penWidth);
    return nullptr;
}
