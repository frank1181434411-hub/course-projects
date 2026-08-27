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
    void onActionOpen();
    void onActionSave();
    void onActionExit();
    void onActionLine();
    void onActionRectangle();
    void onActionEllipse();
    void onActionDiamond();
    void onActionSelect();
    void onActionColor();
    void onActionPenWidth();
    void onActionClear();
    void onActionDeleteSelected();
private:
    void createMenuBar();
    void createToolBar();
    void createStatusBar();
    void updateStatusBar();
    CanvasWidget* canvas;
    QLabel* statusLabel;
    QString currentDrawMode="None";
    QColor currentColor=Qt::black;
    int currentPenWidth=1;
};
#endif //MAIN_WINDOW_H
