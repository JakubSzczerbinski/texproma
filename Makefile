SDL_CFLAGS = $(shell sdl2-config --cflags) 
SDL_LDLIBS = $(shell sdl2-config --libs)
LIBEDIT_CFLAGS = $(shell pkg-config --cflags libedit)
LIBEDIT_LDLIBS = $(shell pkg-config --libs libedit) 
AVCALL_CFLAGS = -Ilibffcall/avcall
AVCALL_LDLIBS = -Llibffcall/avcall/.libs -lavcall

CC	= gcc -g
CFLAGS	= -O2 -Wall \
	  $(SDL_CFLAGS) $(LIBEDIT_CFLAGS) $(AVCALL_CFLAGS) -Ilibtexproma
LDLIBS	= $(SDL_LDLIBS) $(LIBEDIT_LDLIBS) $(AVCALL_LDLIBS) -Llibtexproma -ltexproma

all:	texproma

cell.o: cell.c cell.h tailq.h
dict.o: dict.c dict.h cell.h tailq.h
interp.o: interp.c interp.h cell.h tailq.h dict.h
main.o: main.c interp.h cell.h tailq.h dict.h
gui.o: gui.c gui.h cell.h tailq.h dict.h

texproma: main.o interp.o dict.o cell.o gui.o
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

clean:
	rm -f texproma 
	rm -r -f *.dSYM
	rm -f *.o *~
	rm -f *.taghl

.PHONY: all clean ctags
