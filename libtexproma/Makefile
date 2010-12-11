CC	= gcc -g
CFLAGS	= -Wall -O2
LIBS	= -lm

SOURCES	= common.c generate.c layers.c distort.c

all:	tags cscope.out libtexproma.a

libtexproma.a: $(subst .c,.o,$(SOURCES))
	ar cr $@ $^
	ranlib $@

Makefile.include: $(SOURCES)
	$(CC) -MM $^ > Makefile.include

-include Makefile.include

cscope.out: $(SOURCES)
	cscope -b $^

tags: $(SOURCES)
	# ctags --extra=+fq --fields=+afmikKlnsStz $^
	ctags $^

clean:
	rm -v -r -f .libs
	rm -v -f *~ *.{o,lo,loT} *.{a,la}
	rm -v -f Makefile.include tags cscope.out

.PHONY: all clean

# vim: ts=8 sw=8
