
all: dancing_sudoku

dancing_sudoku: dancing_sudoku.c dancing_links.c dancing_links.h
	gcc -ggdb -O3 -o dancing_sudoku dancing_links.c dancing_sudoku.c -lrt

clean: FORCE
	rm -f dancing_sudoku
	
.PHONY: FORCE

