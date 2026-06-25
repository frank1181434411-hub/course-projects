#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H
#include<QMainWindow>
#include<QLabel>
#include<QColor>

class CanvasWidget;

class MainWindow:public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget* parent=nullptr);
    ~MainWindow();

private slots:
    void onActionNew();
    void onActionExit();
    void onActionRoad();
    void onActionPolygon();
    void onActionPointTest();
    void onActionHermite();
    void onActionBezier();
    void onActionBSpline();
    void onActionEditPoints();
    void onActionColor();
    void onActionPenWidth();
    void onActionRoadWidth();
    void onActionClear();
    void onCanvasStatusMessage(const QString& message);

private:
    void createMenuBar();
    void createStatusBar();
    void updateStatusBar();

    CanvasWidget* canvas;
    QLabel* statusLabel;
    QString currentDrawMode="None";
    QString canvasMessage;
    QColor currentColor=Qt::black;
    int currentPenWidth=2;
    double currentRoadWidth=40.0;
    int currentBezierDegree=3;
    int currentBSplineDegree=3;
};

#endif //MAIN_WINDOW_H
