CSRCS=testKeyboard.cpp KeyboardHandler.cpp findNodeVisitor.cpp

CFLAGS = -g -c -O2 -DLINUX

INCLUDE = -I. -I/usr/include/ -I/usr/include/X11/ -I/usr/local/include/GL 
INCOSG = -I/usr/local/OSG/OSG2.0/OpenSceneGraph/include
LDLIBS = -lm -ldl -lGL -lGLU -lpthread -lXext -lX11

LDFLAGS =  -L. -L/usr/lib -L/usr/X11R6/lib -L/usr/local/lib
LDOSG = -L/usr/local/OSG/OSG2.0/OpenSceneGraph/lib/Linux32 -losg -losgViewer -losgSim
COBJS=$(patsubst %.cpp,%.o,$(CSRCS))

CC = g++

EXE=osex

all: $(EXE)

$(EXE): $(COBJS) 
	cc -o$(EXE) $(COBJS) $(INCLUDE) $(INCOSG) $(LDFLAGS) $(LDLIBS) $(LDOSG)


$(COBJS): %.o : %.cpp 
	$(CC) $(CFLAGS) $(INCLUDE) $(INCOSG) -o $@ $< 


