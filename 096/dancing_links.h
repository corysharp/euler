#ifndef H_dancing_links_h
#define H_dancing_links_h

typedef struct ptrstack {
    void** stack;
    int capacity;
    int size;
} ptrstack_t;

void ptrstack_init( ptrstack_t* self, int initial_capacity );
void ptrstack_add_last( ptrstack_t* self, void* p );
void ptrstack_remove_last( ptrstack_t* self );
void* ptrstack_peek_last( ptrstack_t* self );
void* ptrstack_poll_last( ptrstack_t* self );

typedef struct dlx_cell dlx_cell_t;
struct dlx_cell {
    dlx_cell_t* down;
    dlx_cell_t* up;
    dlx_cell_t* right;
    dlx_cell_t* left;
    dlx_cell_t* column;
    int size;
    const char* name;
};

void dlx_cell_init( dlx_cell_t* self );
void dlx_cell_add_left( dlx_cell_t* self, dlx_cell_t* c );
void dlx_cell_add_right( dlx_cell_t* self, dlx_cell_t* c );
void dlx_cell_add_up( dlx_cell_t* self, dlx_cell_t* c );
void dlx_cell_add_down( dlx_cell_t* self, dlx_cell_t* c );
void dlx_cell_start_new_row( dlx_cell_t* self, dlx_cell_t* column );
void dlx_cell_append( dlx_cell_t* self, dlx_cell_t* c );
void dlx_cell_append_to_row( dlx_cell_t* self, dlx_cell_t* tailcolumn, dlx_cell_t* tail );

typedef struct dlx dlx_t;
struct dlx {
    dlx_cell_t root;
    ptrstack_t visit;
    int max_solutions;
    int num_solutions;
};

void dlx_init( dlx_t* self );
void dlx_preset( dlx_t* self, dlx_cell_t* r );
void dlx_clear( dlx_t* self );
void dlx_cover( dlx_t* self, dlx_cell_t* c );
void dlx_uncover( dlx_t* self, dlx_cell_t* c );

typedef void (*dlx_found_solution_t)( dlx_t* dlx );
void dlx_search( dlx_t* self, dlx_found_solution_t found_solution );

#endif//H_dancing_links_h

