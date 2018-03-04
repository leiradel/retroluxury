SOEXT=.so
TARGET=test$(SOEXT)
OBJEXT=.o
CC=gcc
CXX=g++
#CCFLAGS=-Wall -O2 --std=c99 -I. -I../src
CCFLAGS=-Wall -O0 -g --std=c99 -fPIC -I. -I../src
CXXFLAGS=$(CCFLAGS)
LDFLAGS=-L../src
LIBS=-lretroluxury

OBJS=libretro.o
RES=res/smile.png res/sketch008.ogg res/bounce.wav

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

%.o: %.c
	$(CC) $(CCFLAGS) -c $< -o $@

%.h: %.tar
	xxd -i $< | sed "s/unsigned/const unsigned/g" > $@

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) -shared $(LDFLAGS) -o $@ $+ $(LIBS)

libretro.o: pack.h

pack.h: pack.tar

pack.tar: $(RES)
	tar --xform='s@.*/@@' -cf $@ $+

clean:
	rm -f $(TARGET) $(OBJS) pack.tar

.PHONY: clean
