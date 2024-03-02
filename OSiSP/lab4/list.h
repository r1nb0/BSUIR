#ifndef LIST_H
#define LIST_H

#include <sys/types.h>

typedef struct node_list {
    char index_worker;
    pid_t pid;
    struct node_list* next;
}node_list;

node_list* constructor_list(pid_t, char);
void push_list(node_list**, pid_t, char);
void display_list(const node_list*);
pid_t pop_list(node_list**);
void clear(node_list**);
pid_t erase_list(node_list**, size_t tum);

#endif //LIST_H

