platform = unix
ifeq ($(shell uname -a),)
	platform = win
else ifneq ($(findstring MINGW,$(shell uname -a)),)
	platform = win
else ifneq ($(findstring win,$(shell uname -a)),)
	platform = win
else ifneq ($(findstring Darwin,$(shell uname -a)),)
	platform = osx
endif

CFLAGS = -O3 -I ~/Develop/lua-5.3.4/src -Wall
LFLAGS = -L ~/Develop/lua-5.3.4/src
LIBS   = -llua

ifneq ($(findstring unix,$(platform)),)
	LIBS += -lm
endif

%.o: %.c
	gcc $(CFLAGS) -I. -c -o $@ $<

all: luai

luai: main.o image.o path.o realpath.o
	gcc $(LFLAGS) -o $@ $+ $(LIBS)

main.o: main.c boot_lua.h

boot_lua.h: boot.lua
	xxd -i $< | sed "s@unsigned@@" > $@

clean:
	rm -f luai main.o image.o path.o realpath.o boot_lua.h
