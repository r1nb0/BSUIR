#include <stdio.h>
#include <locale.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#define INITIAL_SIZE 7
#define MAX_LEN_FILE_STR 255
#define ENVIRONMENT_VALUE_CHILD "CHILD_PATH"
#define NAME_BUTTON 2
#define NAME_PROGRAM 0
#define PATH_ENVIRONMENT_FILE 1

bool get_variables(const char* path, char*** buffer) {
    FILE* file = NULL;
    if ((file = fopen(path, "r")) != NULL) {
        size_t ind = 0;
        size_t cnt = INITIAL_SIZE;
        while(cnt--) {
            (*buffer)[ind] = (char*)realloc((*buffer)[ind], (MAX_LEN_FILE_STR + 1) * sizeof(char));
            fscanf(file, "%s", (*buffer)[ind++]);
        }
        fclose(file);
        return true;
    }
    perror("File error");
    return false;
}

bool find_child_path_envp(char** envp, char** path_child) {
    size_t ind = 0;
    size_t len = strlen(ENVIRONMENT_VALUE_CHILD);
    bool flag_find = false;
    while(envp[ind]) {
        flag_find = true;
        for (size_t i = 0; i < len; ++i) {
            if (envp[ind][i] != ENVIRONMENT_VALUE_CHILD[i]) {
                flag_find = false;
                break;
            }
        }
        if (flag_find == true)
            break;
        ind++;
    }
    if (flag_find == false) return false;
    printf("%s\n", envp[ind]);
    size_t path_len = strlen(ENVIRONMENT_VALUE_CHILD);
    size_t find_str_len = strlen(envp[ind]);
    *path_child = (char*)malloc(find_str_len - (path_len+1) + 1);
    size_t j = 0;
    for (size_t i = path_len + 1; i < find_str_len; ++i) {
        (*path_child)[j++] = envp[ind][i];
    }
    path_child[ind] = '\0';
    return true;
}

void increment_child_xx(char** file_name) {
    //child_00
    if ((*file_name)[7] == '9') {
        (*file_name)[7] = '0';
        (*file_name)[6]++;
        return;
    }
    (*file_name)[7]++;
}

int main(int argc, char* argv[], char* envp[]) {
    if (argc == 2) {
        char* path = argv[PATH_ENVIRONMENT_FILE];
        char** array_variables = (char**)calloc(INITIAL_SIZE, sizeof(char*));
        bool flag_alloc_memory = false;
        if(get_variables(path, &array_variables)) {
            flag_alloc_memory = true;
            for (size_t i = 0; i < INITIAL_SIZE; ++i)
                    printf("%s\n", array_variables[i]);
            char* child_name = (char*)malloc(9);
            strcpy(child_name, "child_00");
            char* newargv[] = { child_name, path, NULL, NULL };
            bool flag_continue = true;
            do {
                int ch = getchar();
                switch(ch) {
                    case '+' : {
                        pid_t pid = fork();
                        newargv[NAME_BUTTON] = "+";
                        if (pid == 0) {
                            const char* path_child = getenv(ENVIRONMENT_VALUE_CHILD);
                            execve(path_child, newargv, envp);
                        }else {
                            increment_child_xx(&child_name);
                        }
                        break;
                    }
                    case '*' : {
                        pid_t pid = fork();
                        if (pid == 0) {
                            newargv[NAME_BUTTON] = "*";
                            char* path_child = NULL;
                            if (find_child_path_envp(envp, &path_child)) {
                                execve(path_child, newargv, envp);
                            }
                        }else {
                            increment_child_xx(&child_name);
                        }
                        break;
                    }
                    case '&' : {
                        pid_t pid = fork();
                        if (pid == 0) {
                            newargv[NAME_BUTTON] = "&";
                            char* path_child = NULL;
                            if (find_child_path_envp(__environ, &path_child)) {
                                execve(path_child, newargv, envp);
                            }
                        }else {
                            increment_child_xx(&child_name);
                        }
                        break;
                    }
                    default: { flag_continue = false; break; }
                }
                getchar();
            }while(flag_continue);
            free(child_name);
        }
        if (flag_alloc_memory) {
            for (size_t i = 0; i < INITIAL_SIZE; ++i)
                free(array_variables[i]);
        }
        free(array_variables);
    }else {
        printf("Launch parameters: parent [filename], "
               "where filename is a file with the names of environment variables.");
    }
    return 0;
}
