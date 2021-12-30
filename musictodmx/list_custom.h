#include <stdlib.h>

typedef struct list_t{
    double value;
    struct list_t *next;
}list;

void delete_last(list *l);

list* add_first(list *l, double p);

void free_all(list *l);
