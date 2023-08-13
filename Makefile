.PHONY: all clean dist install uninstall
.SUFFIXES: .c .o

include config.mk

LDADD = -lSDL2 -lSDL2_ttf -lSDL2_image -lSDL2_mixer

BIN = tetris

OBJS = ${SRCS:.c=.o}

SRCS = tetris.c

DATA = sprites.png music.ogg sounds/

all: ${BIN}

clean:
	rm -f ${BIN} ${OBJS}

dist: clean
	mkdir -p ${PACKAGE}-${VERSION}
	cp -Rf Makefile config.mk ${SRC} ${DATA} ${PACKAGE}-${VERSION}
	tar -cf ${PACKAGE}-${VERSION}.tar ${PACKAGE}-${VERSION}
	gzip ${PACKAGE}-${VERSION}.tar
	rm -rf ${PACKAGE}-${VERSION}

install: all
	mkdir -p ${DESTDIR}${PREFIX}${BINDIR}
	cp -f ${BINS} ${DESTDIR}${PREFIX}${BINDIR}
	cd ${DESTDIR}${PREFIX}${BINDIR}
	chmod 755 ${BINS}

uninstall:
	cd ${DESTDIR}${PREFIX}${BINDIR}
	rm -f ${BINS}

${BIN}: ${OBJS}
	${CC} ${LDFLAGS} -o $@ $^ ${LDADD}

.c.o:
	${CC} ${CFLAGS} -c -o $@ $<
