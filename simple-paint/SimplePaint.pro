QT+=core gui widgets

CONFIG+=debug
CONFIG+=console
CONFIG+=c++23

QMAKE_CXXFLAGS+=-std=c++23

TARGET=SimplePaint
TEMPLATE=app

SOURCES+=\
    main.cpp\
    main_window.cpp\
    canvas_widget.cpp\
    shape.cpp\
    exshape.cpp\
    shape_saved.cpp\
    shape_io.cpp

HEADERS+=\
    main_window.h\
    canvas_widget.h\
    shape.h\
    exshape.h\
    shape_saved.h\
    shape_io.h