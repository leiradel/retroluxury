INCLUDES = -I. -I../src
DEFINES =
CFLAGS = -Wall -fPIC $(INCLUDES) $(DEFINES)

ifneq ($(DEBUG),)
  CFLAGS += -O0 -g
else
  CFLAGS += -O3 -DNDEBUG
endif

CXXFLAGS = $(CFLAGS)
LDFLAGS=-L../src

LIBS=-lretroluxury

OBJS=libretro.o
RES=res/smile.png res/sketch008.ogg res/b10.bdf

%.o: %.c
	gcc $(CFLAGS) -std=c99 -c $< -o $@

%.o: %.cpp
	g++ $(CXXFLAGS) -std=c++11 -c $< -o $@

all: test.so pack.zip

test.so: $(OBJS)
	$(CXX) -shared $(LDFLAGS) -o $@ $+ $(LIBS)

pack.zip: $(RES)
	zip -9j $@ $+

clean:
	rm -f test.so $(OBJS) pack.zip

.PHONY: clean
