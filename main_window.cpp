#include"main_window.h"
#include"canvas_widget.h"
#include"shape_saved.h"
#include"shape_io.h"
#include<QVBoxLayout>
#include<QMenuBar>
#include<QToolBar>
#include<QStatusBar>
#include<QAction>
#include<QColorDialog>
#include<QInputDialog>
#include<QSpinBox>
#include<QDialog>
#include<QLabel>
#include<QFileDialog>
#include<QMessageBox>
MainWindow::MainWindow(QWidget* parent):QMainWindow(parent),canvas(nullptr),statusLabel(nullptr)
{
    setWindowTitle("SimplePaint");
    setGeometry(100,100,800,600);
    //Create canvas
    canvas=new CanvasWidget(this);
    setCentralWidget(canvas);
    //Create UI
    createMenuBar();
    createToolBar();
    createStatusBar();
    updateStatusBar();
}
MainWindow::~MainWindow(){}
void MainWindow::createMenuBar()
{
    QMenuBar* menuBar=new QMenuBar(this);
    setMenuBar(menuBar);
    QMenu* fileMenu=menuBar->addMenu("File");
    QAction* actionNew=fileMenu->addAction("New");
    connect(actionNew,&QAction::triggered,this,&MainWindow::onActionNew);
    QAction* actionOpen=fileMenu->addAction("Open");
    connect(actionOpen,&QAction::triggered,this,&MainWindow::onActionOpen);
    QAction* actionSave=fileMenu->addAction("Save");
    connect(actionSave,&QAction::triggered,this,&MainWindow::onActionSave);
    fileMenu->addSeparator();
    QAction* actionExit=fileMenu->addAction("Exit");
    connect(actionExit,&QAction::triggered,this,&MainWindow::onActionExit);
    QMenu* drawMenu=menuBar->addMenu("Draw");
    QAction* actionLine=drawMenu->addAction("Line");
    connect(actionLine,&QAction::triggered,this,&MainWindow::onActionLine);
    QAction* actionRect=drawMenu->addAction("Rectangle");
    connect(actionRect,&QAction::triggered,this,&MainWindow::onActionRectangle);
    QAction* actionEllipse=drawMenu->addAction("Ellipse");
    connect(actionEllipse,&QAction::triggered,this,&MainWindow::onActionEllipse);
    QAction* actionDiamond=drawMenu->addAction("Diamond");
    connect(actionDiamond,&QAction::triggered,this,&MainWindow::onActionDiamond);
    QAction* actionSelect=drawMenu->addAction("Select");
    connect(actionSelect,&QAction::triggered,this,&MainWindow::onActionSelect);
    QMenu* editMenu=menuBar->addMenu("Edit");
    QAction* actionClear=editMenu->addAction("Clear");
    connect(actionClear,&QAction::triggered,this,&MainWindow::onActionClear);
    QAction* actionDeleteSelected=editMenu->addAction("Delete Selected");
    connect(actionDeleteSelected,&QAction::triggered,this,&MainWindow::onActionDeleteSelected);
    QMenu* styleMenu=menuBar->addMenu("Style");
    QAction* actionColor=styleMenu->addAction("Color");
    connect(actionColor,&QAction::triggered,this,&MainWindow::onActionColor);
    QAction* actionPenWidth=styleMenu->addAction("Pen Width");
    connect(actionPenWidth,&QAction::triggered,this,&MainWindow::onActionPenWidth);
}
void MainWindow::createToolBar()
{
    QToolBar* toolBar=addToolBar("Tools");
    QAction* actionLine=toolBar->addAction("Line");
    connect(actionLine,&QAction::triggered,this,&MainWindow::onActionLine);
    QAction* actionRect=toolBar->addAction("Rect");
    connect(actionRect,&QAction::triggered,this,&MainWindow::onActionRectangle);
    QAction* actionEllipse=toolBar->addAction("Ellipse");
    connect(actionEllipse,&QAction::triggered,this,&MainWindow::onActionEllipse);
    QAction* actionDiamond=toolBar->addAction("Diamond");
    connect(actionDiamond,&QAction::triggered,this,&MainWindow::onActionDiamond);
    QAction* actionSelect=toolBar->addAction("Select");
    connect(actionSelect,&QAction::triggered,this,&MainWindow::onActionSelect);
    toolBar->addSeparator();
    QAction* actionColor=toolBar->addAction("Color");
    connect(actionColor,&QAction::triggered,this,&MainWindow::onActionColor);
    QAction* actionPenWidth=toolBar->addAction("Pen Width");
    connect(actionPenWidth,&QAction::triggered,this,&MainWindow::onActionPenWidth);
    toolBar->addSeparator();
    QAction* actionClear=toolBar->addAction("Clear");
    connect(actionClear,&QAction::triggered,this,&MainWindow::onActionClear);
    QAction* actionDeleteSelected=toolBar->addAction("Delete Selected");
    connect(actionDeleteSelected,&QAction::triggered,this,&MainWindow::onActionDeleteSelected);
}
void MainWindow::createStatusBar()
{
    statusLabel=new QLabel("Mode: None | Color: Black | Width: 1",this);
    statusBar()->addWidget(statusLabel);
}
void MainWindow::updateStatusBar()
{
    QString text=QString("Mode: %1 | Color: RGB(%2,%3,%4) | Width: %5")
        .arg(currentDrawMode)
        .arg(currentColor.red())
        .arg(currentColor.green())
        .arg(currentColor.blue())
        .arg(currentPenWidth);
    statusLabel->setText(text);
}
void MainWindow::onActionNew() {canvas->clearCanvas();}
void MainWindow::onActionOpen()
{
    QString fileName=QFileDialog::getOpenFileName(this,"Open Drawing","","Text Files (*.txt)");
    if(fileName.isEmpty()) return;
    ShapeIO::loadFromFile(*canvas->getShapeSaved(),fileName);
    canvas->clearSelection();
    canvas->update();
    QMessageBox::information(this,"Success","Drawing loaded successfully!");
}
void MainWindow::onActionSave()
{
    QString fileName=QFileDialog::getSaveFileName(this,"Save Drawing","","Text Files (*.txt)");
    if(fileName.isEmpty()) return;
    if(ShapeIO::saveToFile(*canvas->getShapeSaved(),fileName))
        QMessageBox::information(this,"Success","Drawing saved successfully!");
    else
        QMessageBox::warning(this,"Error","Failed to save drawing!");
}
void MainWindow::onActionExit() {close();}
void MainWindow::onActionLine()
{
    canvas->setDrawMode(DrawMode::Line);
    currentDrawMode="Line";
    updateStatusBar();
}
void MainWindow::onActionRectangle()
{
    canvas->setDrawMode(DrawMode::Rectangle);
    currentDrawMode="Rectangle";
    updateStatusBar();
}
void MainWindow::onActionEllipse()
{
    canvas->setDrawMode(DrawMode::Ellipse);
    currentDrawMode="Ellipse";
    updateStatusBar();
}
void MainWindow::onActionDiamond()
{
    canvas->setDrawMode(DrawMode::Diamond);
    currentDrawMode="Diamond";
    updateStatusBar();
}
void MainWindow::onActionSelect()
{
    canvas->setDrawMode(DrawMode::Select);
    currentDrawMode="Select";
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
void MainWindow::onActionClear(){canvas->clearCanvas();}
void MainWindow::onActionDeleteSelected(){canvas->deleteSelectedShape();}
