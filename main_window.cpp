#include"main_window.h"
#include"canvas_widget.h"
#include<QMenuBar>
#include<QStatusBar>
#include<QAction>
#include<QActionGroup>
#include<QColorDialog>
#include<QInputDialog>
#include<QLabel>

MainWindow::MainWindow(QWidget* parent):QMainWindow(parent),canvas(nullptr),statusLabel(nullptr)
{
    setWindowTitle("Computer Graphics Course Design");
    setGeometry(100,100,1000,700);
    canvas=new CanvasWidget(this);
    setCentralWidget(canvas);
    connect(canvas,&CanvasWidget::statusMessageChanged,this,&MainWindow::onCanvasStatusMessage);
    createMenuBar();
    createStatusBar();
    updateStatusBar();
}

MainWindow::~MainWindow(){}

void MainWindow::createMenuBar()
{
    QMenuBar* menuBar=new QMenuBar(this);
    setMenuBar(menuBar);

    QMenu* roadMenu=menuBar->addMenu("Road");
    QAction* actionRoad=roadMenu->addAction("Draw Road");
    connect(actionRoad,&QAction::triggered,this,&MainWindow::onActionRoad);
    roadMenu->addSeparator();
    QAction* actionRoadWidth=roadMenu->addAction("Road Width");
    connect(actionRoadWidth,&QAction::triggered,this,&MainWindow::onActionRoadWidth);
    QAction* actionRoadColor=roadMenu->addAction("Color");
    connect(actionRoadColor,&QAction::triggered,this,&MainWindow::onActionColor);
    QAction* actionRoadPenWidth=roadMenu->addAction("Pen Width");
    connect(actionRoadPenWidth,&QAction::triggered,this,&MainWindow::onActionPenWidth);

    QMenu* polygonMenu=menuBar->addMenu("Polygon");
    QAction* actionPolygon=polygonMenu->addAction("Draw Polygon");
    connect(actionPolygon,&QAction::triggered,this,&MainWindow::onActionPolygon);
    QAction* actionPointTest=polygonMenu->addAction("Point Test");
    connect(actionPointTest,&QAction::triggered,this,&MainWindow::onActionPointTest);
    polygonMenu->addSeparator();
    QAction* actionPolygonColor=polygonMenu->addAction("Color");
    connect(actionPolygonColor,&QAction::triggered,this,&MainWindow::onActionColor);
    QAction* actionPolygonPenWidth=polygonMenu->addAction("Pen Width");
    connect(actionPolygonPenWidth,&QAction::triggered,this,&MainWindow::onActionPenWidth);

    QMenu* curveMenu=menuBar->addMenu("Curve");
    QAction* actionHermite=curveMenu->addAction("Hermite Curve");
    connect(actionHermite,&QAction::triggered,this,&MainWindow::onActionHermite);

    QMenu* bezierMenu=curveMenu->addMenu("Bezier Curve");
    QAction* actionBezier=bezierMenu->addAction("Draw Bezier");
    connect(actionBezier,&QAction::triggered,this,&MainWindow::onActionBezier);
    QMenu* bezierDegreeMenu=bezierMenu->addMenu("Degree");
    QActionGroup* bezierDegreeGroup=new QActionGroup(this);
    bezierDegreeGroup->setExclusive(true);
    for(int degree=1;degree<=5;++degree)
    {
        QAction* action=bezierDegreeMenu->addAction(QString("Degree %1").arg(degree));
        action->setCheckable(true);
        action->setData(degree);
        if(degree==currentBezierDegree) action->setChecked(true);
        bezierDegreeGroup->addAction(action);
    }
    connect(bezierDegreeGroup,&QActionGroup::triggered,this,[this](QAction* action)
    {
        currentBezierDegree=action->data().toInt();
        canvas->setBezierDegree(currentBezierDegree);
        updateStatusBar();
    });

    QMenu* bSplineMenu=curveMenu->addMenu("B-Spline Curve");
    QAction* actionBSpline=bSplineMenu->addAction("Draw B-Spline");
    connect(actionBSpline,&QAction::triggered,this,&MainWindow::onActionBSpline);
    QMenu* bSplineDegreeMenu=bSplineMenu->addMenu("Degree");
    QActionGroup* bSplineDegreeGroup=new QActionGroup(this);
    bSplineDegreeGroup->setExclusive(true);
    for(int degree=2;degree<=3;++degree)
    {
        QAction* action=bSplineDegreeMenu->addAction(QString("Degree %1").arg(degree));
        action->setCheckable(true);
        action->setData(degree);
        if(degree==currentBSplineDegree) action->setChecked(true);
        bSplineDegreeGroup->addAction(action);
    }
    connect(bSplineDegreeGroup,&QActionGroup::triggered,this,[this](QAction* action)
    {
        currentBSplineDegree=action->data().toInt();
        canvas->setBSplineDegree(currentBSplineDegree);
        updateStatusBar();
    });

    curveMenu->addSeparator();
    QAction* actionCurveColor=curveMenu->addAction("Color");
    connect(actionCurveColor,&QAction::triggered,this,&MainWindow::onActionColor);
    QAction* actionCurvePenWidth=curveMenu->addAction("Pen Width");
    connect(actionCurvePenWidth,&QAction::triggered,this,&MainWindow::onActionPenWidth);

    QAction* actionClear=menuBar->addAction("Clear");
    connect(actionClear,&QAction::triggered,this,&MainWindow::onActionClear);

    QAction* actionEditPoints=menuBar->addAction("Edit Points");
    connect(actionEditPoints,&QAction::triggered,this,&MainWindow::onActionEditPoints);
}

void MainWindow::createStatusBar()
{
    statusLabel=new QLabel(this);
    statusBar()->addWidget(statusLabel);
}

void MainWindow::updateStatusBar()
{
    QString text=QString("Current mode: %1 | Color: RGB(%2,%3,%4) | Pen width: %5 | Road width: %6 | Bezier degree: %7 | B-Spline degree: %8")
        .arg(currentDrawMode)
        .arg(currentColor.red())
        .arg(currentColor.green())
        .arg(currentColor.blue())
        .arg(currentPenWidth)
        .arg(currentRoadWidth)
        .arg(currentBezierDegree)
        .arg(currentBSplineDegree);
    if(!canvasMessage.isEmpty())
        text=text+" | "+canvasMessage;
    statusLabel->setText(text);
}

void MainWindow::onActionNew(){canvas->clearCanvas();}
void MainWindow::onActionExit(){close();}

void MainWindow::onActionRoad()
{
    canvas->setDrawMode(DrawMode::Road);
    currentDrawMode="Road";
    updateStatusBar();
}

void MainWindow::onActionPolygon()
{
    canvas->setDrawMode(DrawMode::Polygon);
    currentDrawMode="Polygon";
    updateStatusBar();
}

void MainWindow::onActionPointTest()
{
    canvas->setDrawMode(DrawMode::PointTest);
    currentDrawMode="Point Test";
    updateStatusBar();
}

void MainWindow::onActionHermite()
{
    canvas->setDrawMode(DrawMode::Hermite);
    currentDrawMode="Hermite Curve";
    updateStatusBar();
}

void MainWindow::onActionBezier()
{
    canvas->setBezierDegree(currentBezierDegree);
    canvas->setDrawMode(DrawMode::Bezier);
    currentDrawMode=QString("Bezier Curve Degree %1").arg(currentBezierDegree);
    updateStatusBar();
}

void MainWindow::onActionBSpline()
{
    canvas->setBSplineDegree(currentBSplineDegree);
    canvas->setDrawMode(DrawMode::BSpline);
    currentDrawMode=QString("B-Spline Curve Degree %1").arg(currentBSplineDegree);
    updateStatusBar();
}

void MainWindow::onActionEditPoints()
{
    canvas->setDrawMode(DrawMode::EditPoints);
    currentDrawMode="Edit Points";
    updateStatusBar();
}

void MainWindow::onActionColor()
{
    QColor color=QColorDialog::getColor(currentColor,this);
    if(color.isValid())
    {
        currentColor=color;
        canvas->setPenColor(color);
        updateStatusBar();
    }
}

void MainWindow::onActionPenWidth()
{
    bool ok;
    int width=QInputDialog::getInt(this,"Pen Width","Enter pen width (1-20):",currentPenWidth,1,20,1,&ok);
    if(ok)
    {
        currentPenWidth=width;
        canvas->setPenWidth(width);
        updateStatusBar();
    }
}

void MainWindow::onActionRoadWidth()
{
    bool ok;
    double width=QInputDialog::getDouble(this,"Road Width","Enter road width:",currentRoadWidth,4.0,200.0,1,&ok);
    if(ok)
    {
        currentRoadWidth=width;
        canvas->setRoadWidth(width);
        updateStatusBar();
    }
}

void MainWindow::onActionClear(){canvas->clearCanvas();}

void MainWindow::onCanvasStatusMessage(const QString& message)
{
    canvasMessage=message;
    updateStatusBar();
}
