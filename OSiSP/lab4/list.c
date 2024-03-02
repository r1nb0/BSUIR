#include "list.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

node_list* constructor_list(pid_t pid, char index) {
    node_list* val = (node_list*)malloc(1 * sizeof(node_list));
    val->next = NULL;
    val->pid = pid;
    val->index_worker = index;
    return val;
}

void push_list(node_list** head, pid_t pid, char index) {
    if (head == NULL)
        exit(-1);
    if (*head ==  NULL) {
        *head = constructor_list(pid, index);
        return;
    }
    node_list* cursor = *head;
    while(cursor->next)
        cursor = cursor->next;
    cursor->next = constructor_list(pid, index);
}

void display_list(const node_list* head) {
    if (head != NULL) {
        printf("Parent PID: %d\n", head->pid);
    }else return;
    if (head->next) {
        head = head->next;
        size_t i = 1;
        while(head) {
            printf("Child_%lu_%c with PID: %d\n", i++, head->index_worker, head->pid);
            head = head->next;
        }
    }
}

pid_t pop_list(node_list** head) {
    if (head == NULL)
        exit(-1);
    if (*head == NULL)
        return -1;
    node_list* cursor = *head;
    node_list* prev = NULL;
    while(cursor -> next) {
        prev = cursor;
        cursor = cursor->next;
    }
    pid_t pid = cursor->pid;
    free(cursor);
    if (prev == NULL)
        *head = NULL;
    else prev -> next = NULL;
    return pid;
}

pid_t erase_list(node_list** head, size_t num) {
    if (head == NULL) {
        exit(-2);
    }
    if (*head == NULL) {
        return -1;
    }
    node_list* prev = NULL;
    node_list* curr = *head;
    size_t i = 0;
    while(curr->next && i != num) {
        prev = curr;
        curr = curr->next;
        ++i;
    }
    if (i != num) {
        printf("There is no element with this number.");
        return -1;
    }
    pid_t pid = curr->pid;
    if (prev == NULL) {
        *head = (*head)->next;
        free(curr);
        return pid;
    }
    prev -> next = curr->next;
    free(curr);
    return pid;
}