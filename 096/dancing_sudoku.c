#include "dancing_links.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>


uint64_t nano_time() {
    struct timespec t;
    clock_gettime( CLOCK_REALTIME, &t );
    return ((uint64_t)t.tv_sec) * 1000000000ULL + ((uint64_t)(t.tv_nsec));
}


typedef struct sudoku_cell {
    dlx_cell_t super;
    int8_t row;
    int8_t col;
    int8_t box;
    int8_t val;
} sudoku_cell_t;

void sudoku_cell_init( sudoku_cell_t* self, int8_t row, int8_t col, int8_t box, int8_t val ) {
    dlx_cell_init( &self->super );
    self->row = row;
    self->col = col;
    self->box = box;
    self->val = val;
}

void sudoku_cell_init_col( sudoku_cell_t* self, int8_t col ) {
    sudoku_cell_init( self, 0, col, 0, 0 );
}

void sudoku_cell_init_cell( sudoku_cell_t* self, sudoku_cell_t* c ) {
    sudoku_cell_init( self, c->row, c->col, c->box, c->val );
}

typedef struct sudoku {
    dlx_t super;
    sudoku_cell_t* cells;
    sudoku_cell_t* cols;
    int nsolved;
    int sum;
    int solution_flags;
} sudoku_t;

enum {
    FLAG_DO_EULER_SUM = 0x01,
    FLAG_DO_PRINT_SOLUTIONS = 0x02,
    FLAG_DO_PRINT_COMPACT_SOLUTIONS = 0x04,
};

void sudoku_init( sudoku_t* self ) {
    int i;
    int n;
    int row;
    int col;
    int val;
    int box;
    sudoku_cell_t* r;

    dlx_init( &self->super );
    self->super.max_solutions = 1;
    self->nsolved = 0;
    self->sum = 0;
    self->solution_flags = 0;
    self->cells = (sudoku_cell_t*)malloc( 4*9*9*9*sizeof(sudoku_cell_t) );
    self->cols = (sudoku_cell_t*)malloc( 4*9*9*sizeof(sudoku_cell_t) );
    for (i=0; i<4*9*9; i++) {
        sudoku_cell_init_col( &self->cols[i], i );
        dlx_cell_append( &self->super.root, &self->cols[i].super );
    }
    for (row=0; row<9; row++) {
        for (col=0; col<9; col++) {
            for (val=0; val<9; val++) {
                box = 3*(row/3) + (col/3);
                n = 81*row+9*col+val;
                r = &self->cells[n];
                sudoku_cell_init( r, row, col, box, val+1 );
                sudoku_cell_init_cell( &self->cells[1*9*9*9+n], r );
                sudoku_cell_init_cell( &self->cells[2*9*9*9+n], r );
                sudoku_cell_init_cell( &self->cells[3*9*9*9+n], r );
                dlx_cell_start_new_row( &r->super, &self->cols[81*0+9*row+col].super );
                dlx_cell_append_to_row( &r->super, &self->cols[81*1+9*row+val].super, &self->cells[1*9*9*9+n].super );
                dlx_cell_append_to_row( &r->super, &self->cols[81*2+9*col+val].super, &self->cells[2*9*9*9+n].super );
                dlx_cell_append_to_row( &r->super, &self->cols[81*3+9*box+val].super, &self->cells[3*9*9*9+n].super );
            }
        }
    }
}

void sudoku_preset( sudoku_t* self, const char* puzzle ) {
    int n;
    char c;
    for (n=0; n<9*9; n++) {
        c = puzzle[n];
        if (c != '0') {
            dlx_preset( &self->super, &self->cells[9*n+c-'1'].super );
        }
    }
}

void sudoku_found_solution( dlx_t* dlx ) {
    sudoku_t* self = (sudoku_t*)dlx;
    self->nsolved++;
    if (self->solution_flags != 0) {
        char sol[9*9];
        int i;
        for (i=0; i<dlx->visit.size; i++) {
            sudoku_cell_t* c = (sudoku_cell_t*)dlx->visit.stack[i];
            sol[9*c->row+c->col] = c->val;
        }
        if ((self->solution_flags & FLAG_DO_EULER_SUM) != 0) {
            self->sum += sol[0]*100 + sol[1]*10 + sol[2];
        }
        if ((self->solution_flags & FLAG_DO_PRINT_SOLUTIONS) != 0) {
            int j;
            for (i=0; i<9; i++) {
                for (j=0; j<9; j++) {
                    printf( "%d", sol[9*i+j] );
                    if ((j%3)==2) {
                        printf( " " );
                    }
                }
                printf( "\n" );
                if ((i%3)==2) {
                    printf( "\n" );
                }
            }
        }
        if ((self->solution_flags & FLAG_DO_PRINT_COMPACT_SOLUTIONS) != 0) {
            int j;
            for (i=0; i<9; i++) {
                for (j=0; j<9; j++) {
                    printf( "%d", sol[9*i+j] );
                }
            }
            printf( "\n" );
        }
    }
}

void sudoku_solve( sudoku_t* self, const char* puzzle ) {
    sudoku_preset( self, puzzle );
    dlx_search( &self->super, sudoku_found_solution );
    dlx_clear( &self->super );
}

int sudoku_main( int argc, char** argv ) {
    bool print_solution = false;
    const char* puzzle_filename = NULL;
    sudoku_t sudoku;
    FILE* fin;
    char line[128];
    int n;
    int nloaded = 0;
    uint64_t t0;
    uint64_t t1;

    sudoku_init( &sudoku );

    for (n=1; n<argc; n++) {
        const char* opt = argv[n];
        if (opt[0] == '-') {
            if (strcmp( opt, "-a" ) == 0) {
                sudoku.super.max_solutions = ((~(unsigned int)0) >> 1);
            }
            else if (strcmp( opt, "-c" ) == 0) {
                sudoku.solution_flags |= FLAG_DO_PRINT_COMPACT_SOLUTIONS;
            }
            else if (strcmp( opt, "-e" ) == 0) {
                sudoku.solution_flags |= FLAG_DO_EULER_SUM;
            }
            else if (strcmp( opt, "-p" ) == 0) {
                sudoku.solution_flags |= FLAG_DO_PRINT_SOLUTIONS;
            }
            else {
                fprintf( stderr, "unknown command line option \"%s\"\n", opt );
                return 1;
            }
        }
        else if (puzzle_filename == NULL) {
            puzzle_filename = opt;
        }
        else {
            fprintf( stderr, "only one puzzle.txt argument may be specified but \"%s\" given\n", opt);
            return 1;
        }
    }

    if (puzzle_filename == NULL) {
        printf("usage: dancing_sudoku [puzzles.txt]\n");
        printf("\n");
        printf("Options:\n");
        printf("    -a  find all solutions, not just the first\n");
        printf("    -c  print compact solutions\n");
        printf("    -e  calculate Project Euler sum\n");
        printf("    -p  print solutions\n");
        printf("\n");
        printf("puzzles.txt must either be in the format given in at Project Euler or must\n");
        printf("contain one puzzle per line, ordered row-first.  Zeros indicate an empty\n");
        printf("square.  For example, this is a valid description of a puzzle:\n");
        printf("\n");
        printf("003020600900305001001806400008102900700000008006708200002609500800203009005010300\n");
        return 0;
    }

    fin = fopen( puzzle_filename, "r" );
    if (fin == NULL) {
        fprintf( stderr, "could not open puzzle file \"%s\"\n", puzzle_filename );
        return 1;
    }

    t0 = nano_time();

    while (fgets( line, sizeof(line), fin ) != NULL) {
        const char* puzzle = NULL;
        char buf[82];
        if (strncmp(line, "Grid", 4) == 0) {
            int i;
            puzzle = buf;
            for (i=0; i<9; i++) {
                if (fgets( line, sizeof(line), fin ) != NULL) {
                    if (strspn( line, "0123456789" ) == 9) {
                        memcpy( buf+i*9, line, 9 );
                    }
                    else {
                        puzzle = NULL;
                        break;
                    }
                }
                else {
                    puzzle = NULL;
                    break;
                }
            }
        }
        else if (strspn(line, "0123456789") == 81) {
            puzzle = line;
        }
        if (puzzle != NULL) {
            nloaded++;
            sudoku_solve( &sudoku, puzzle );
        }
    }

    t1 = nano_time();

    if ((sudoku.solution_flags & FLAG_DO_EULER_SUM) != 0) {
        printf( "Project Euler sum %d\n", sudoku.sum );
    }
    printf( "%d out of %d puzzles solved\n", sudoku.nsolved, nloaded );
    printf( "%.6f seconds\n", (t1-t0)/1e9 );
    printf( "%.1f puzzles/second\n", (nloaded*1e9)/(t1-t0) );
    printf( "%.3f microseconds/puzzle\n", (t1-t0)/(nloaded*1e3) );
    return 0;
}

int main( int argc, char** argv ) {
    exit( sudoku_main( argc, argv ) );
    return 1;
}

