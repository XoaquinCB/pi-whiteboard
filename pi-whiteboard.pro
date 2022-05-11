DESTDIR = build
OBJECTS_DIR = build/.obj
MOC_DIR = build/.moc
RCC_DIR = build/.rcc
UI_DIR = build/.ui

LIBS += -lwiringPi -pthread

HEADERS = src/*.hpp src/*.h
SOURCES = src/*.cpp

QT = widgets
