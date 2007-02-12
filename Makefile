CC = gcc
CFLAGS = -ansi -pedantic -Wall -Werror
LDFLAGS = `sdl-config --cflags --libs`
IN = src
OUT = bin
OBJS = opentyr.o newshape.o fonthand.o

all: tyrian

.PHONY: clean
clean:
	-rm ${IN}/*.o
	-rm ${OUT}/tyrian
	-rmdir ${OUT}

tyrian: ${OBJS}
	-mkdir ${OUT} 
	${CC} ${LDFLAGS} -o ${OUT}/$@ $(OBJS:%=${IN}/%)

${OBJS}:
	${CC} ${CFLAGS} -c ${IN}/$*.c -o ${IN}/$*.o