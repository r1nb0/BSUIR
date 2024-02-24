#include "list.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>


node* constructor_list(pid_t pid) {
    node* val = (node*)malloc(1 * sizeof(node));
    val->next = NULL;
    val->pid = pid;
    return val;
}

void push_list(node** head, pid_t pid) {
    if (head == NULL)
        exit(-1);
    if (*head ==  NULL) {
        *head = constructor_list(pid);
        return;
    }
    node* cursor = *head;
    while(cursor->next)
        cursor = cursor->next;
    cursor->next = constructor_list(pid);
}

void display_list(const node* head) {
    if (head != NULL) {
        printf("Parent PID: %d\n", head->pid);
    }else return;
    if (head->next) {
        head = head->next;
        size_t i = 1;
        while(head) {
            printf("Child%lu PID: %d\n", i++, head->pid);
            head = head->next;
        }
    }
}

pid_t pop_list(node** head) {
    if (head == NULL)
        exit(-1);
    if (*head == NULL)
        return -1;
    node* cursor = *head;
    node* prev = NULL;
    while(cursor -> next) {
        prev = cursor;
        cursor = cursor->next;
    }
    pid_t pid = cursor->pid;
    free(cursor);
    if (!prev)
        *head = NULL;
    else prev -> next = NULL;
    return pid;
}
