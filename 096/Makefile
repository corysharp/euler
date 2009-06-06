CFLAGS += -ggdb
CFLAGS += -O3
LIBS += -lrt

all: dancing_sudoku

dancing_sudoku: dancing_sudoku.c dancing_links.c dancing_links.h
	gcc $(CFLAGS) -o $@ dancing_links.c dancing_sudoku.c $(LIBS)

clean: FORCE
	rm -f dancing_sudoku
	
.PHONY: FORCE

