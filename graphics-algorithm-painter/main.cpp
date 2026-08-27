#include<QApplication>
#include"main_window.h"
int main(int argc,char* argv[])
{
    QApplication app(argc,argv);
    MainWindow window;
    window.show();
    return app.exec();
}
/*
cd D:\CPP\QT\graphics-algorithm-painter
D:\msys2\ucrt64\bin\qmake6.exe graphics-algorithm-painter.pro -o Makefile
D:\msys2\ucrt64\bin\mingw32-make.exe
$env:PATH="D:\msys2\ucrt64\bin;$env:PATH"
.\debug\graphics-algorithm-painter.exe
*/
