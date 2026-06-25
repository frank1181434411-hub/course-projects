# graphics-algorithm-painter

本项目是一个基于 Qt Widgets 的计算机图形学课程设计程序，程序框架参考
[frank1181434411-hub/simple-paint](https://github.com/frank1181434411-hub/simple-paint)。

项目保留了原画板程序的窗口、画布、鼠标交互和图形对象管理思路，并将功能改造成三个图形学算法实验：

- 折线路径道路生成
- 多边形与点的位置关系判定
- Hermite、Bezier、B-Spline 曲线生成与交互

## 构建运行

```powershell
cd D:\CPP\QT\graphics-algorithm-painter
D:\msys2\ucrt64\bin\qmake6.exe graphics-algorithm-painter.pro -o Makefile
D:\msys2\ucrt64\bin\mingw32-make.exe
$env:PATH="D:\msys2\ucrt64\bin;$env:PATH"
.\debug\graphics-algorithm-painter.exe
```

## 道路生成

道路功能位于 `road_shape.h / road_shape.cpp`。

用户通过鼠标输入一条折线中心线，程序根据道路总宽 `roadWidth` 计算左右边界。设相邻中心点为 `Pi` 和 `Pi+1`，线段方向为：

```cpp
QPointF direction=centerPoints[i+1]-centerPoints[i];
QPointF unit=normalizeVector(direction);
```

单位法向量为：

```cpp
QPointF normal(-unit.y()*side,unit.x()*side);
```

由于 `roadWidth` 表示道路总宽，实际偏移距离使用半宽：

```cpp
double halfWidth=roadWidth/2.0;
offsets.push_back(QPointF(-unit.y()*side,unit.x()*side)*halfWidth);
```

每个端点直接按对应线段法向量偏移：

```cpp
boundary.push_back(centerPoints.front()+offsets.front());
boundary.push_back(centerPoints.back()+offsets.back());
```

中间折点处，程序采用偏移线求交。对折点 `Pi`，前一段和后一段分别得到两个偏移点：

```cpp
QPointF previousOffsetPoint=centerPoints[i]+offsets[i-1];
QPointF nextOffsetPoint=centerPoints[i]+offsets[i];
```

再沿前后线段方向构造两条偏移直线，求交作为该侧边界点：

```cpp
QPointF previousDirection=centerPoints[i]-centerPoints[i-1];
QPointF nextDirection=centerPoints[i+1]-centerPoints[i];
QPointF a2=previousOffsetPoint+previousDirection;
QPointF b2=nextOffsetPoint+nextDirection;

if(lineIntersection(previousOffsetPoint,a2,nextOffsetPoint,b2,cross))
    boundary.push_back(cross);
else
    boundary.push_back((previousOffsetPoint+nextOffsetPoint)*0.5);
```

绘制时不填充道路区域，只显示左右边界、中心线和控制点：

```cpp
for(int i=1;i<static_cast<int>(left.size());++i)
    painter.drawLine(left[i-1],left[i]);
for(int i=1;i<static_cast<int>(right.size());++i)
    painter.drawLine(right[i-1],right[i]);
```

这样可以更直观地观察折线扩展后的左右边界。

## 多边形与点的位置关系

多边形功能位于 `polygon_shape.h / polygon_shape.cpp`，几何工具函数位于 `geometry_utils.h / geometry_utils.cpp`。

用户先输入多边形顶点，再切换到点测试模式。程序判断点与多边形的关系：

```cpp
enum class PointRelation
{
    Inside,
    Outside,
    Boundary
};
```

判定顺序是：

1. 先判断点是否在多边形边界上。
2. 若不在边界上，再使用射线法判断内外。

边界判断使用点到线段距离：

```cpp
if(distancePointToSegment(point,a,b)>eps) return false;
```

同时检查点是否落在线段包围盒内：

```cpp
return point.x()>=minX && point.x()<=maxX && point.y()>=minY && point.y()<=maxY;
```

射线法核心逻辑如下：

```cpp
bool crosses=(a.y()>point.y())!=(b.y()>point.y());
if(crosses)
{
    double x=(b.x()-a.x())*(point.y()-a.y())/(b.y()-a.y())+a.x();
    if(x>point.x()) inside=!inside;
}
```

如果水平射线与多边形边相交次数为奇数，则点在内部；为偶数，则点在外部。

最终封装在：

```cpp
PointRelation classifyPointInPolygon(const QPointF& point,const std::vector<QPointF>& polygon,double eps)
```

`PolygonShape::classifyPoint()` 直接调用该函数完成判定。

## 曲线生成

曲线功能位于 `curve_shape.h / curve_shape.cpp`。

程序支持三类曲线：

- Hermite 曲线
- Bezier 曲线
- B-Spline 曲线

### Hermite 曲线

Hermite 曲线使用 4 个控制点，当前输入顺序为：

```text
P0
T0Control
P1
T1Control
```

其中 `P0` 和 `P1` 是起点与终点，两个方向向量由端点指向方向控制点得到：

```cpp
QPointF p0=controlPoints[0];
QPointF p1=controlPoints[2];
QPointF t0=controlPoints[1]-p0;
QPointF t1=controlPoints[3]-p1;
```

曲线点使用 Hermite 基函数生成：

```cpp
double h00=2*t3-3*t2+1;
double h01=-2*t3+3*t2;
double h10=t3-2*t2+t;
double h11=t3-t2;
return p0*h00+p1*h01+t0*h10+t1*h11;
```

### Bezier 曲线

Bezier 曲线使用 De Casteljau 算法。程序支持 1 到 5 次 Bezier 曲线，次数由菜单选择，`n` 次曲线需要 `n+1` 个控制点。

核心算法是不断对相邻控制点做线性插值：

```cpp
std::vector<QPointF> temp;
for(int i=0;i<=degree && i<static_cast<int>(controlPoints.size());++i)
    temp.push_back(controlPoints[i]);
for(int level=static_cast<int>(temp.size())-1;level>0;--level)
    for(int i=0;i<level;++i)
        temp[i]=lerpPoint(temp[i],temp[i+1],t);
return temp.front();
```

线性插值函数为：

```cpp
QPointF lerpPoint(const QPointF& a,const QPointF& b,double t)
{
    return QPointF(a.x()+(b.x()-a.x())*t,a.y()+(b.y()-a.y())*t);
}
```

### B-Spline 曲线

B-Spline 支持二次和三次均匀 B 样条。

二次 B-Spline 每 3 个相邻控制点生成一段：

```cpp
double b0=(1-2*t+t2)/2.0;
double b1=(-2*t2+2*t+1)/2.0;
double b2=t2/2.0;
return p0*b0+p1*b1+p2*b2;
```

三次 B-Spline 每 4 个相邻控制点生成一段：

```cpp
double b0=(-t3+3*t2-3*t+1)/6.0;
double b1=(3*t3-6*t2+4)/6.0;
double b2=(-3*t3+3*t2+3*t+1)/6.0;
double b3=t3/6.0;
return p0*b0+p1*b1+p2*b2+p3*b3;
```

程序根据当前选择的次数自动决定每段曲线使用的控制点数量：

```cpp
int segmentCount=static_cast<int>(controlPoints.size())-degree;
```

## 控制点编辑

程序提供 `Edit Points` 模式。该模式下可以拖动已有道路、多边形和曲线的控制点。

所有图形都通过 `Shape` 基类暴露统一接口：

```cpp
virtual int controlPointCount() const;
virtual QPointF controlPointAt(int index) const;
virtual void moveControlPoint(int index,const QPointF& point);
virtual int hitControlPoint(const QPointF& point,double radius) const;
```

画布命中控制点后，调用：

```cpp
draggedShape->moveControlPoint(draggedControlPointIndex,point);
```

图形会在下一次重绘时根据新的控制点重新计算。
