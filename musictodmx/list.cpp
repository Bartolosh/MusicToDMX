#include "list.h"

void delete_last(list *l){
    
    list *prev;

    while(l->next == NULL){
        prev = l;
        l = l->next;
    }
    free(prev->next);
    prev->next = NULL;
}

list* add_first(list *l, double p){
    list *first = (list*)malloc(sizeof(list));
    first->value = p; 
    first->next = l;
    return first;
}

void free_all(list *l){
    free_all(l->next);
    free(l);
}
