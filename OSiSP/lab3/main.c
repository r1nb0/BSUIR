#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include "list.h"

#define COUNT_OF_REPEATS 4

typedef struct pair {
    int first;
    int second;
}pair;

size_t cnt = 0;
size_t size = 0;
pair arr_of_statistic[COUNT_OF_REPEATS];
pair val_statistic;
bool continue_collect = true;
bool collect = true;
bool flag_p = false;
pid_t pid_parent;
node* head;

void allow_statistic_output();
void disable_statistic_output();
void show_statistic();
void take_statistic();
void delete_last_proc();
void make_statistic();
void create_proc();
void delete_all_child_proc();
void command_to_stat_for_n_proc(size_t, bool, bool);
void command_to_show_stat_for_all_proc(bool);
void allow_all_after_p();


void print_hello() {
    printf("HELLO");
}

int main() {
    int num = 0;
    signal(SIGINT, show_statistic);
    signal(SIGUSR1, allow_statistic_output);
    signal(SIGUSR2, disable_statistic_output);
    bool flag_continue = true;
    pid_parent = getpid();
    push_list(&head, pid_parent);
    do {
        int ch = getchar();
        switch(ch) {
            case '+' : { create_proc(); break; }
            case '-' : { delete_last_proc(); break; }
            case 'l' : { display_list(head); break;}
            case 'k' : { delete_all_child_proc(); break; }
            case 's' : {
                printf("Enter 0 for all processes, or another number for <num> process: ");
                scanf("%d", &num);
                if (num == 0)
                    command_to_show_stat_for_all_proc(false);
                else command_to_stat_for_n_proc(num, false, false);
                break;
            }
            case 'g' : {
                flag_p = false;
                printf("Enter 0 for all processes, or another number for <num> process: ");
                scanf("%d", &num);
                if (num == 0)
                    command_to_show_stat_for_all_proc(true);
                else command_to_stat_for_n_proc(num, true, false);
                break;
            }
            case 'p' : {
                command_to_show_stat_for_all_proc(false);
                printf("Enter the number of the process that will display statistics: ");
                scanf("%d", &num);
                command_to_stat_for_n_proc(num, true, true);
                command_to_stat_for_n_proc(num, false, false);
                flag_p = true;
                signal(SIGALRM, allow_all_after_p);
                alarm(5);

                break;
            }
            case 'q' : { delete_all_child_proc(), flag_continue = false; break; }
            default : { flag_continue = false; break; }
        }
        getchar();
    }while(flag_continue);
    return 0;
}

void command_to_stat_for_n_proc(size_t n, bool allow_flag, bool query_flag) {
    if (cnt >= n) {
        size_t i = 1;
        node* cursor = head->next;
        while(i++ != n)
            cursor = cursor->next;
        if (query_flag) {
            kill(cursor->pid, SIGINT);
            return;
        }
        if (allow_flag)
            kill(cursor->pid, SIGUSR1);
        kill(cursor->pid, SIGUSR2);
    }else printf("There is no child process with this number.\n");
}


void allow_statistic_output() {
    collect = true;
}

void disable_statistic_output() {
    collect = false;
}

void show_statistic() {
    printf("Statistic of child process with PID = %d, PPID = %d All values: ", getpid(), getppid());
    for (size_t i = 0; i < size; ++i) {
        printf("{%d, %d} ", arr_of_statistic[i].first, arr_of_statistic[i].second);
    }
    printf("\n");
}

void take_statistic() {
    arr_of_statistic[size].first = val_statistic.first;
    arr_of_statistic[size++].second = val_statistic.second;
    continue_collect = false;
}

void delete_last_proc() {
    pid_t pid = pop_list(&head);
    kill(pid, SIGKILL);
    printf("Child process with PID = %d successfully deleted.\n", pid);
    printf("Remaining number of child processes: %lu\n", --cnt);
}

void make_statistic() {
    do {
        size = 0;
        for (size_t i = 0; i < COUNT_OF_REPEATS; ++i) {
            alarm(3);
            size_t j = 0;
            do {
                if (j % 2 == 0) {
                    val_statistic.first = 0;
                    val_statistic.second = 0;
                }else {
                    val_statistic.first = 1;
                    val_statistic.second = 1;
                }
                j++;
            }while(continue_collect);
            continue_collect = true;
        }
        if (collect)
          show_statistic();
    }while(1);
}

void create_proc() {
    pid_t pid = fork();
    if (pid == 0) {
        signal(SIGALRM, take_statistic);
        make_statistic();
    }
    else if (pid > 0) {
        push_list(&head, pid);
        cnt++;
        printf("Child process with PID = %d spawned successfully.\n", pid);
    }
}

void command_to_show_stat_for_all_proc(bool allow_flag) {
    if (head->next) {
        node* cursor = head->next;
        while(cursor) {
            if (allow_flag)
                kill(cursor->pid, SIGUSR1);
            else kill(cursor->pid, SIGUSR2);
            cursor = cursor->next;
        }
    }
}

void delete_all_child_proc() {
   while(head->next) {
       pid_t pid = pop_list(&head);
       kill(pid, SIGKILL);
       cnt--;
   }
    printf("All child processes are deleted.\n");
}

void allow_all_after_p() {
    if (flag_p == true)
        command_to_show_stat_for_all_proc(true);
}
