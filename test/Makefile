SOEXT=.so
TARGET=test$(SOEXT)
OBJEXT=.o
CC=gcc
CXX=g++
#CCFLAGS=-Wall -O2 --std=c99 -I. -I../src
CCFLAGS=-Wall -O0 -g --std=c99 -fPIC -I. -I../src
CXXFLAGS=$(CCFLAGS)
LDFLAGS=-L../src
LIBS=-lretroluxury -lgcc_s -lgcc

OBJS=libretro.o
RES=res/smile.png res/sketch008.ogg res/bounce.wav res/b10.bdf

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

%.o: %.c
	$(CC) $(CCFLAGS) -c $< -o $@

%.h: %.tar
	xxd -i $< | sed "s/unsigned/const unsigned/g" > $@

all: $(TARGET) pack.zip

$(TARGET): $(OBJS)
	$(CXX) -shared $(LDFLAGS) -o $@ $+ $(LIBS)

pack.zip: $(RES)
	zip -9j $@ $+

clean:
	rm -f $(TARGET) $(OBJS) pack.zip

.PHONY: clean
