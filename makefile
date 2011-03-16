TYPE = APP

NAME = WaveView

SRCS = App.cpp
SRCS += MainWindow.cpp
SRCS += MainView.cpp
LIBS = be media stdc++

RDEFS = WaveView.rdef

# include the makefile-engine
include $(BUILDHOME)/etc/makefile-engine
