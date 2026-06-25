QT+=core gui widgets

CONFIG+=debug
CONFIG+=console
CONFIG+=c++23

QMAKE_CXXFLAGS+=-std=c++23

TARGET=graphics-algorithm-painter
TEMPLATE=app

SOURCES+=\
    main.cpp\
    main_window.cpp\
    canvas_widget.cpp\
    shape.cpp\
    shape_saved.cpp\
    geometry_utils.cpp\
    road_shape.cpp\
    polygon_shape.cpp\
    curve_shape.cpp

HEADERS+=\
    main_window.h\
    canvas_widget.h\
    shape.h\
    shape_saved.h\
    geometry_utils.h\
    road_shape.h\
    polygon_shape.h\
    curve_shape.h
