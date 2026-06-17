# SimplePaint 课程设计说明文档

## 一、项目简介

SimplePaint 是一个基于 Qt Widgets 和 C++ 编写的简易绘图程序。程序支持基本图形绘制、颜色设置、画笔宽度设置、画布清空、图形选择、Ctrl 多选、删除选中图形、保存绘图结果和重新打开绘图文件等功能。

项目采用面向对象方式设计图形系统。公共图形属性和统一接口由抽象基类 `Shape` 提供，直线、矩形、椭圆和菱形分别由派生类实现。主窗口 `MainWindow` 负责菜单栏、工具栏、状态栏和用户命令分发；画布控件 `CanvasWidget` 负责鼠标交互、图形预览、图形选择和绘制刷新；`ShapeSaved` 负责维护图形对象集合；`ShapeIO` 负责将图形数据保存到文本文件或从文本文件读取回来。

当前版本已经删除独立的 `CircleShape` 和 `Circle` 工具。正圆不再作为单独图形类存在，而是通过在 `Ellipse` 工具下按住 `Shift` 绘制得到。

## 二、开发环境

本项目使用 Qt 工程文件 `SimplePaint.pro` 管理构建配置。

主要环境要求：

- Qt 模块：`core`、`gui`、`widgets`
- C++ 标准：C++23
- 工程类型：Qt Widgets 桌面应用程序
- 构建方式：qmake 工程
- 目标程序名：`SimplePaint`

`SimplePaint.pro` 中的主要配置：

```pro
QT+=core gui widgets

CONFIG+=debug
CONFIG+=console
CONFIG+=c++23

QMAKE_CXXFLAGS+=-std=c++23

TARGET=SimplePaint
TEMPLATE=app
```

## 三、项目文件结构

核心文件如下：

```text
SimplePaint/
|-- SimplePaint.pro        Qt 工程配置文件
|-- main.cpp               程序入口
|-- main_window.h          主窗口类声明
|-- main_window.cpp        主窗口类实现
|-- canvas_widget.h        绘图画布控件声明
|-- canvas_widget.cpp      绘图画布控件实现
|-- shape.h                图形抽象基类声明
|-- shape.cpp              图形抽象基类实现
|-- exshape.h              具体图形类声明
|-- exshape.cpp            具体图形类实现
|-- shape_saved.h          图形集合管理类声明
|-- shape_saved.cpp        图形集合管理类实现
|-- shape_io.h             图形文件读写类声明
|-- shape_io.cpp           图形文件读写类实现
`-- README.md              项目说明文档
```

`Makefile`、`Makefile.Debug`、`Makefile.Release`、`.qmake.stash` 等文件属于 qmake 构建过程中产生或使用的辅助文件。

## 四、编译与运行

### 方式一：使用 Qt Creator

1. 打开 Qt Creator。
2. 选择 `Open Project`。
3. 打开项目目录中的 `SimplePaint.pro`。
4. 配置合适的 Qt Kit。
5. 点击构建并运行。

### 方式二：使用命令行

进入项目目录：

```bash
cd QT/SimplePaint
```

执行 qmake：

```bash
qmake SimplePaint.pro
```

如果使用 MinGW：

```bash
mingw32-make
```

如果使用 MSVC 对应的 nmake 环境：

```bash
nmake
```

构建完成后运行生成的 `SimplePaint` 可执行程序即可。

如果前一次构建后修改过带 `Q_OBJECT` 的类，例如 `MainWindow` 或 `CanvasWidget`，建议重新运行 qmake 或执行 Clean/Rebuild，让 Qt 自动生成的 moc 文件刷新到最新状态。

## 五、功能说明

### 1. 图形绘制

程序当前支持以下四种基础图形：

- 直线：`Line`
- 矩形：`Rectangle`
- 椭圆：`Ellipse`
- 菱形：`Diamond`

用户可以通过菜单栏 `Draw` 菜单选择绘图模式，也可以通过工具栏按钮快速选择。

基本绘图流程：

1. 选择一种绘图工具。
2. 在白色画布上按下鼠标左键确定起点。
3. 拖动鼠标形成预览图形。
4. 松开鼠标后，将当前预览图形保存到图形集合中。

画布在拖动过程中会实时显示预览图形。鼠标释放后，预览图形会变成正式保存的图形。

### 2. Shift 约束绘制

按住 `Shift` 拖动时，部分工具会进入约束绘制模式。

当前实现如下：

- `Rectangle` + `Shift`：绘制正方形。
- `Ellipse` + `Shift`：绘制正圆。
- `Diamond` + `Shift`：绘制基于正方形外接框的菱形。
- `Line` + `Shift`：当前未做角度约束，仍按普通直线绘制。

正圆不再由单独的 `CircleShape` 实现，而是由 `EllipseShape` 在宽高相等时表现出来。因此保存文件中也不会再生成新的 `Circle` 类型记录。

### 3. 图形选择

程序提供 `Select` 选择工具。

进入选择模式的方法：

- 菜单栏：`Draw -> Select`
- 工具栏：`Select`

选择模式下，点击图形边缘附近即可选中该图形。被选中的图形会显示红色虚线外框。

选择检测由每个图形类自己的 `contains()` 函数完成：

- `LineShape`：判断鼠标点到线段的距离是否足够近。
- `RectShape`：判断鼠标点是否靠近矩形四条边。
- `EllipseShape`：判断鼠标点是否靠近椭圆边缘。
- `DiamondShape`：判断鼠标点是否靠近菱形四条边。

当多个图形重叠时，选择会从后绘制的图形开始检测，因此更容易选中视觉上位于上层的图形。

### 4. Ctrl 多选

选择模式下支持 `Ctrl` 多选。

具体规则：

- 普通点击某个图形：清空旧选择，只选中当前图形。
- 按住 `Ctrl` 点击未选中的图形：将该图形加入已选集合。
- 按住 `Ctrl` 点击已经选中的图形：取消选中该图形。
- 普通点击空白处：清空当前选择。
- 按住 `Ctrl` 点击空白处：保留当前选择。

内部使用 `std::vector<int> selectedShapeIndexes` 保存当前被选中的图形下标。绘制时逐个判断图形是否在选中集合中，如果在，则额外绘制红色虚线选择框。

### 5. 删除选中图形

随机删除功能已经删除。当前删除功能是删除用户选中的具体图形。

使用方式：

1. 切换到 `Select` 模式。
2. 点击选择一个图形，或按住 `Ctrl` 选择多个图形。
3. 点击菜单栏 `Edit -> Delete Selected`，或点击工具栏 `Delete Selected`。

删除多个图形时，程序会先将选中下标从大到小排序，再逐个删除，避免删除前面的图形后影响后续下标。

如果当前没有选中任何图形，执行 `Delete Selected` 不会产生效果。

### 6. 颜色设置

通过菜单栏 `Style -> Color` 或工具栏 `Color` 按钮可以打开颜色选择对话框。

选择颜色后：

- 当前画笔颜色会更新。
- 后续绘制的图形使用新颜色。
- 状态栏中的 RGB 颜色值会同步更新。

默认颜色为黑色。

### 7. 画笔宽度设置

通过菜单栏 `Style -> Pen Width` 或工具栏 `Pen Width` 按钮可以设置画笔宽度。

程序使用 `QInputDialog::getInt` 输入整数宽度，取值范围为：

```text
1 ~ 20
```

默认画笔宽度为 `1`。

### 8. 新建和清空画布

菜单栏 `File -> New` 会调用清空画布功能。

菜单栏 `Edit -> Clear` 和工具栏 `Clear` 按钮也会清空当前画布。

清空画布时，程序会删除当前保存的所有图形对象，清空选中状态，并刷新画布显示。

### 9. 保存绘图

菜单栏 `File -> Save` 会打开保存文件对话框。

保存对话框使用的过滤器为：

```text
Text Files (*.txt)
```

需要注意的是，程序当前只是通过对话框提示用户保存为 `.txt` 文本文件，并没有在代码中强制追加 `.txt` 后缀。如果用户输入不带后缀的文件名，程序会按用户输入的路径保存。

保存成功后，程序弹出成功提示；保存失败时，程序弹出错误提示。

### 10. 打开绘图文件

菜单栏 `File -> Open` 会打开文件选择对话框。

打开对话框同样使用：

```text
Text Files (*.txt)
```

打开文件后，程序会清空当前图形集合，并按文件内容逐行解析图形数据。解析成功的图形会加入画布，随后刷新显示并清空选中状态。

### 11. 退出程序

菜单栏 `File -> Exit` 会关闭主窗口并退出程序。

## 六、界面组成

主窗口类为 `MainWindow`，继承自 `QMainWindow`。

窗口初始化时完成以下操作：

1. 设置窗口标题为 `SimplePaint`。
2. 设置窗口初始位置和大小为 `800 x 600`。
3. 创建 `CanvasWidget` 作为中心控件。
4. 创建菜单栏。
5. 创建工具栏。
6. 创建状态栏。
7. 更新状态栏信息。

### 菜单栏

菜单栏包含四组菜单。

#### File

- `New`：清空画布。
- `Open`：打开绘图文件。
- `Save`：保存绘图文件。
- `Exit`：退出程序。

#### Draw

- `Line`：切换到直线绘制模式。
- `Rectangle`：切换到矩形绘制模式。
- `Ellipse`：切换到椭圆绘制模式。
- `Diamond`：切换到菱形绘制模式。
- `Select`：切换到图形选择模式。

#### Edit

- `Clear`：清空画布。
- `Delete Selected`：删除当前选中的一个或多个图形。

#### Style

- `Color`：选择画笔颜色。
- `Pen Width`：设置画笔宽度。

### 工具栏

工具栏提供常用操作按钮：

- `Line`
- `Rect`
- `Ellipse`
- `Diamond`
- `Select`
- `Color`
- `Pen Width`
- `Clear`
- `Delete Selected`

工具栏按钮与菜单栏中的对应功能连接到同一组槽函数。

### 状态栏

状态栏显示当前绘图状态，格式如下：

```text
Mode: 当前模式 | Color: RGB(r,g,b) | Width: 当前线宽
```

例如：

```text
Mode: Line | Color: RGB(0,0,0) | Width: 1
```

## 七、核心类设计

### 1. MainWindow

`MainWindow` 是程序主窗口类，负责整体界面和用户命令处理。

主要成员变量：

- `CanvasWidget* canvas`：中心绘图画布。
- `QLabel* statusLabel`：状态栏文本标签。
- `QString currentDrawMode`：当前绘图模式名称。
- `QColor currentColor`：当前画笔颜色。
- `int currentPenWidth`：当前画笔宽度。

主要成员函数：

- `createMenuBar()`：创建菜单栏。
- `createToolBar()`：创建工具栏。
- `createStatusBar()`：创建状态栏。
- `updateStatusBar()`：刷新状态栏内容。

主要槽函数：

- `onActionNew()`：清空画布。
- `onActionOpen()`：打开绘图文件。
- `onActionSave()`：保存绘图文件。
- `onActionExit()`：退出程序。
- `onActionLine()`：选择直线模式。
- `onActionRectangle()`：选择矩形模式。
- `onActionEllipse()`：选择椭圆模式。
- `onActionDiamond()`：选择菱形模式。
- `onActionSelect()`：选择图形选择模式。
- `onActionColor()`：设置画笔颜色。
- `onActionPenWidth()`：设置画笔宽度。
- `onActionClear()`：清空画布。
- `onActionDeleteSelected()`：删除选中的图形。

### 2. CanvasWidget

`CanvasWidget` 是绘图画布类，继承自 `QWidget`。

它负责处理鼠标事件、保存绘图状态、生成预览图形、执行 Shift 约束绘制、维护选中状态并绘制所有图形。

主要成员变量：

- `std::unique_ptr<ShapeSaved> shapeSaved`：图形集合管理对象。
- `DrawMode currentMode`：当前绘图模式。
- `QColor penColor`：当前画笔颜色。
- `int penWidth`：当前画笔宽度。
- `bool isDrawing`：是否正在绘图。
- `QPoint startPoint`：鼠标按下时的起点。
- `QPoint currentEndPoint`：当前鼠标位置或释放位置。
- `std::unique_ptr<Shape> previewShape`：绘图过程中的预览图形。
- `std::vector<int> selectedShapeIndexes`：当前被选中的图形下标集合。

重写的事件函数：

- `mousePressEvent(QMouseEvent* event)`：鼠标按下，负责开始绘图或执行选择。
- `mouseMoveEvent(QMouseEvent* event)`：鼠标移动，更新终点并生成预览图形。
- `mouseReleaseEvent(QMouseEvent* event)`：鼠标释放，保存最终图形。
- `paintEvent(QPaintEvent* event)`：绘制背景、已有图形、选中框和预览图形。

辅助函数：

- `createPreviewShape()`：根据当前绘图模式创建对应图形对象。
- `finalizeShape()`：将预览图形加入保存集合。
- `clearCanvas()`：清空画布。
- `deleteSelectedShape()`：删除当前选中的图形。
- `clearSelection()`：清空选中状态。
- `selectShapeAt()`：根据鼠标点击位置选择图形，支持 Ctrl 多选。
- `drawSelection()`：绘制选中图形的红色虚线外框。
- `applyShiftConstraint()`：在按住 Shift 时修正终点，使矩形、椭圆和菱形按正形约束绘制。
- `isShapeSelected()`：判断指定下标图形是否处于选中状态。

### 3. Shape

`Shape` 是所有图形的抽象基类。

它保存所有图形共有的数据：

- `QPoint startPoint`：起点。
- `QPoint endPoint`：终点。
- `QColor color`：颜色。
- `int penWidth`：线宽。

它定义了四个纯虚函数：

- `draw(QPainter& painter) const`：绘制图形。
- `typeName() const`：返回图形类型名称。
- `saveText() const`：返回用于保存到文件的文本内容。
- `contains(const QPoint& point) const`：判断一个点是否命中当前图形。

该设计使程序可以通过基类指针统一管理不同类型的图形对象。

### 4. 具体图形类

具体图形类定义在 `exshape.h` 和 `exshape.cpp` 中。

包括：

- `LineShape`
- `RectShape`
- `EllipseShape`
- `DiamondShape`

这些类都继承自 `Shape`，并分别实现自己的绘制逻辑、保存文本格式和命中检测逻辑。

#### LineShape

使用 `QPainter::drawLine` 根据起点和终点绘制直线。命中检测通过计算鼠标点到线段的距离完成。

#### RectShape

通过起点和终点计算矩形左上角坐标、宽度和高度，然后使用 `QPainter::drawRect` 绘制矩形。命中检测会判断鼠标点是否靠近矩形四条边。

按住 `Shift` 绘制矩形时，画布会先把终点修正到宽高相等的位置，因此最终得到正方形。

#### EllipseShape

通过起点和终点确定外接矩形，然后使用 `QPainter::drawEllipse` 绘制椭圆。命中检测会判断鼠标点是否靠近椭圆边缘。

按住 `Shift` 绘制椭圆时，画布会先把外接矩形约束为正方形，因此最终得到正圆。

#### DiamondShape

根据起点和终点确定外接矩形，计算上、右、下、左四个顶点，再使用 `QPolygon` 和 `QPainter::drawPolygon` 绘制菱形。命中检测会判断鼠标点是否靠近菱形四条边。

按住 `Shift` 绘制菱形时，画布会先把外接矩形约束为正方形，因此最终得到基于正方形外接框的标准菱形。

### 5. ShapeSaved

`ShapeSaved` 用于保存和管理当前画布中的图形对象。

内部使用：

```cpp
std::vector<std::unique_ptr<Shape>> shapes;
```

这样可以保存不同派生类图形对象，同时通过 `std::unique_ptr` 自动管理内存。

主要功能：

- `addShape()`：添加图形。
- `removeShapeAt()`：按下标删除指定图形。
- `clearAllShapes()`：清空所有图形。
- `getShapes()`：获取图形集合。
- `getShapeCount()`：获取图形数量。

### 6. ShapeIO

`ShapeIO` 负责图形文件保存和读取。

它提供三个静态函数：

- `saveToFile()`：将图形集合保存到文件。
- `loadFromFile()`：从文件读取图形集合。
- `parseShapeLine()`：解析单行文本并创建对应图形对象。

文件保存时，程序遍历所有图形对象，调用每个图形的 `saveText()` 函数，将图形数据逐行写入文本文件。

文件读取时，程序逐行读取文本，根据第一项图形类型创建对应图形对象。

## 八、绘图流程说明

完整绘图流程如下：

1. 用户通过菜单栏或工具栏选择绘图模式。
2. `MainWindow` 调用 `canvas->setDrawMode()` 设置当前模式。
3. 用户在画布按下鼠标。
4. `CanvasWidget::mousePressEvent()` 记录起点并设置 `isDrawing=true`。
5. 用户拖动鼠标。
6. 如果按住 `Shift` 且当前工具是矩形、椭圆或菱形，`applyShiftConstraint()` 会修正当前终点。
7. `CanvasWidget::mouseMoveEvent()` 调用 `createPreviewShape()` 创建预览图形。
8. 画布调用 `update()` 触发重绘。
9. `paintEvent()` 绘制所有已保存图形、选中框和当前预览图形。
10. 用户松开鼠标。
11. `mouseReleaseEvent()` 使用最终终点调用 `finalizeShape()`，将预览图形加入 `ShapeSaved`。
12. 画布再次刷新，最终图形显示在画布上。

## 九、选择和删除流程说明

完整选择和删除流程如下：

1. 用户选择 `Select` 工具。
2. 用户点击画布。
3. `CanvasWidget::selectShapeAt()` 从后往前遍历图形集合，调用各图形的 `contains()` 判断是否命中。
4. 如果普通点击命中图形，则清空旧选择并只选中当前图形。
5. 如果按住 `Ctrl` 点击命中图形，则切换当前图形的选中状态。
6. `paintEvent()` 为所有选中图形绘制红色虚线外框。
7. 用户点击 `Delete Selected`。
8. `CanvasWidget::deleteSelectedShape()` 将选中下标排序去重，从大到小删除对应图形。
9. 删除完成后清空选中状态并刷新画布。

## 十、保存文件格式

项目使用纯文本格式保存图形。

每一行表示一个图形，格式如下：

```text
图形类型 起点x 起点y 终点x 终点y 红色值 绿色值 蓝色值 线宽
```

例如：

```text
Line 10 20 200 100 0 0 0 1
Rect 50 60 180 140 255 0 0 3
Ellipse 100 100 260 180 0 128 255 2
Diamond 200 120 320 240 180 80 220 2
```

支持的图形类型字符串：

- `Line`
- `Rect`
- `Ellipse`
- `Diamond`

读取文件时，程序会根据每行开头的类型字符串判断要创建哪种图形对象。如果类型无法识别，当前行不会生成图形对象。

选中状态、Ctrl 多选状态和当前工具状态都属于运行时交互状态，不会保存到文件中。

## 十一、程序特点

### 1. 面向对象结构清晰

图形基类 `Shape` 定义统一接口，各种具体图形继承并实现自己的绘制、保存和命中检测逻辑。新增图形时，只需要新增派生类，并在画布创建逻辑和文件解析逻辑中增加对应分支。

### 2. 使用智能指针管理对象

程序使用 `std::unique_ptr<Shape>` 保存图形对象，避免手动释放内存，降低内存泄漏风险。

### 3. 使用 Qt 事件机制处理绘图交互

画布通过重写鼠标事件和绘制事件实现交互式绘图，符合 Qt Widgets 程序的常见设计方式。

### 4. 文件格式简单直观

保存格式为纯文本，每行一个图形，便于调试、查看和手动理解。

### 5. 菜单栏和工具栏功能一致

菜单栏和工具栏都连接到同一组槽函数，减少重复逻辑。

### 6. 支持更接近常见绘图软件的操作方式

程序支持 `Shift` 约束绘制正方形、正圆和标准菱形，也支持 `Ctrl` 多选后批量删除图形，交互方式更接近常见绘图软件。

## 十二、可扩展方向

后续可以在当前结构基础上继续扩展：

- 增加撤销和重做功能。
- 增加橡皮擦功能。
- 增加选中后移动、缩放图形功能。
- 增加填充颜色。
- 增加更多图形类型，例如三角形、多边形、自由曲线。
- 为直线工具增加 `Shift` 角度约束，例如水平、垂直、45 度斜线。
- 保存时自动补全 `.txt` 后缀。
- 增加图片导出功能，例如导出为 PNG。
- 增加异常文件格式提示。
- 增加快捷键，例如 `Ctrl+S` 保存、`Ctrl+O` 打开。

## 十三、当前注意事项

1. 保存对话框使用 `.txt` 文件过滤器，但代码没有强制文件后缀。
2. 打开文件时会先清空当前画布。
3. 文件解析以空格分隔字段，因此保存文件中的每个字段应保持空格分隔。
4. 当前保存格式不再生成 `Circle` 类型；正圆保存为宽高相等的 `Ellipse`。
5. 当前图形只绘制边框，没有填充颜色。
6. 当前画布背景固定为白色。

## 十四、总结

SimplePaint 实现了一个完整的 Qt 简易绘图程序，包含窗口界面、鼠标绘图、Shift 约束绘制、图形选择、Ctrl 多选、删除选中图形、图形对象管理、文件保存读取和状态栏显示等功能。项目整体结构清晰，类之间职责明确，适合作为 Qt Widgets、C++ 面向对象编程、事件处理和简单文件序列化的课程设计示例。
