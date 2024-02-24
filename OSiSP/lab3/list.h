#ifndef LIST_H
#define LIST_H

#include <sys/types.h>

typedef struct node {
    pid_t pid;
    struct node* next;
}node;

node* constructor_list(pid_t);
void push_list(node**, pid_t);
void display_list(const node*);
pid_t pop_list(node** pid_t);
void clear(node**);

#endif //LIST_H
