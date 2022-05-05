DESTDIR = build
OBJECTS_DIR = build/.obj
MOC_DIR = build/.moc
RCC_DIR = build/.rcc
UI_DIR = build/.ui

QMAKE_CXXFLAGS += -lwiringPi

HEADERS = src/*.hpp
SOURCES = src/*.cpp

QT = widgets
