#include "dancing_links.h"
#include <stdlib.h>
#include <string.h>

void ptrstack_init( ptrstack_t* self, int initial_capacity ) {
    self->size = 0;
    self->capacity = initial_capacity;
    self->stack = (void**)malloc( self->capacity * sizeof(void*) );
    memset( self->stack, 0, self->capacity * sizeof(void*) );
}

void ptrstack_add_last( ptrstack_t* self, void* p ) {
    if (self->size >= self->capacity) {
        self->stack = realloc( self->stack, 2 * self->capacity * sizeof(void*) );
        memset( self->stack + self->capacity, 0, self->capacity * sizeof(void*) );
        self->capacity *= 2;
    }
    self->stack[ self->size++ ] = p;
}

void ptrstack_remove_last( ptrstack_t* self ) {
    if (self->size > 0) {
        self->size--;
    }
}

void* ptrstack_peek_last( ptrstack_t* self ) {
    if (self->size > 0) {
        return self->stack[ self->size - 1 ];
    }
    else {
        return NULL;
    }
}

void* ptrstack_poll_last( ptrstack_t* self ) {
    if (self->size > 0) {
        return self->stack[ --self->size ];
    }
    else {
        return NULL;
    }
}

void dlx_cell_init( dlx_cell_t* self ) {
    self->down = self;
    self->up = self;
    self->right = self;
    self->left = self;
    self->column = self;
    self->size = 0;
    self->name = NULL;
}

void dlx_cell_add_left( dlx_cell_t* self, dlx_cell_t* c ) {
    c->left = self->left;
    c->right = self;
    self->left->right = c;
    self->left = c;
}

void dlx_cell_add_right( dlx_cell_t* self, dlx_cell_t* c ) {
    c->right = self->right;
    c->left = self;
    self->right->left = c;
    self->right = c;
}

void dlx_cell_add_up( dlx_cell_t* self, dlx_cell_t* c ) {
    c->up = self->up;
    c->down = self;
    self->up->down = c;
    self->up = c;
}

void dlx_cell_add_down( dlx_cell_t* self, dlx_cell_t* c ) {
    c->down = self->down;
    c->up = self;
    self->down->up = c;
    self->down = c;
}

void dlx_cell_start_new_row( dlx_cell_t* self, dlx_cell_t* column ) {
    self->left = self;
    self->right = self;
    self->column = column;
    dlx_cell_add_up( column, self );
    column->size++;
}

void dlx_cell_append( dlx_cell_t* self, dlx_cell_t* c ) {
    dlx_cell_add_left( self, c );
}

void dlx_cell_append_to_row( dlx_cell_t* self, dlx_cell_t* tailcolumn, dlx_cell_t* tail ) {
    dlx_cell_start_new_row( tail, tailcolumn );
    dlx_cell_append( self, tail );
}

void dlx_init( dlx_t* self ) {
    dlx_cell_init( &self->root );
    ptrstack_init( &self->visit, 100 );
    self->max_solutions = ((~(unsigned int)0) >> 1);
    self->num_solutions = 0;
}

void dlx_preset( dlx_t* self, dlx_cell_t* r ) {
    dlx_cell_t* j;
    dlx_cover( self, r->column );
    for (j=r->right; j!=r; j=j->right) {
        dlx_cover( self, j->column );
    }
    ptrstack_add_last( &self->visit, r );
}

void dlx_clear( dlx_t* self ) {
    dlx_cell_t* r;
    dlx_cell_t* j;
    while ((r = (dlx_cell_t*)ptrstack_poll_last( &self->visit )) != NULL) {
        for (j=r->left; j!=r; j=j->left) {
            dlx_uncover( self, j->column );
        }
        dlx_uncover( self, r->column );
    }
    self->num_solutions = 0;
}

void dlx_cover( dlx_t* self, dlx_cell_t* c ) {
    dlx_cell_t* i;
    dlx_cell_t* j;
    c->right->left = c->left;
    c->left->right = c->right;
    for (i=c->down; i!=c; i=i->down) {
        for (j=i->right; j!=i; j=j->right) {
            j->down->up = j->up;
            j->up->down = j->down;
            j->column->size--;
        }
    }
}

void dlx_uncover( dlx_t* self, dlx_cell_t* c ) {
    dlx_cell_t* i;
    dlx_cell_t* j;
    for (i=c->up; i!=c; i=i->up) {
        for (j=i->left; j!=i; j=j->left) {
            j->down->up = j;
            j->up->down = j;
            j->column->size++;
        }
    }
    c->right->left = c;
    c->left->right = c;
}

#if 0
void dlx_search( dlx_t* self, dlx_found_solution_t found_solution ) {
    dlx_cell_t* c = self->root.right;
    dlx_cell_t* j;
    dlx_cell_t* r;
    if (c == &self->root) {
        self->num_solutions++;
        found_solution( self );
    }
    else {
        for (j=c->right; j!=&self->root; j=j->right) {
            if (j->size < c->size)
                c = j;
        }
        if (c->size > 0) {
            dlx_cover( self, c );
            for (r=c->down; r!=c; r=r->down) {
                for (j=r->right; j!=r; j=j->right) {
                    dlx_cover( self, j->column );
                }
                ptrstack_add_last( &self->visit, r );
                dlx_search( self, found_solution );
                ptrstack_remove_last( &self->visit );
                for (j=r->left; j!=r; j=j->left) {
                    dlx_uncover( self, j->column );
                }

                if (self->num_solutions >= self->max_solutions)
                    break;
            }
            dlx_uncover( self, c );
        }
    }
}
#endif

#if 1
void dlx_search( dlx_t* self, dlx_found_solution_t found_solution ) {
    dlx_cell_t* c;
    dlx_cell_t* j;
    dlx_cell_t* r;
    dlx_cell_t* last_preset = ptrstack_peek_last( &self->visit );
dlx_search_recurse:
    c = self->root.right;
    if (c == &self->root) {
        self->num_solutions++;
        found_solution( self );
    }
    else {
        for (j=c->right; j!=&self->root; j=j->right) {
            if (j->size < c->size)
                c = j;
        }
        if (c->size > 0) {
            dlx_cover( self, c );
            for (r=c->down; r!=c; r=r->down) {
                for (j=r->right; j!=r; j=j->right) {
                    dlx_cover( self, j->column );
                }
                ptrstack_add_last( &self->visit, r );
                goto dlx_search_recurse;
dlx_search_resume:
                r = (dlx_cell_t*)ptrstack_poll_last( &self->visit );
                if (r == last_preset) {
                    if (last_preset != NULL) {
                        self->visit.size++; //undo poll_last
                    }
                    return;
                }
                c = r->column;
                for (j=r->left; j!=r; j=j->left) {
                    dlx_uncover( self, j->column );
                }

                if (self->num_solutions >= self->max_solutions)
                    break;
            }
            dlx_uncover( self, r->column );
        }
    }
    goto dlx_search_resume;
}
#endif

