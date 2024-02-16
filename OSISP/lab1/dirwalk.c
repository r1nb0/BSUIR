#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <string.h>
#include <getopt.h>
#include <stdbool.h>
#include <locale.h>

enum OPTIONS {
    IS_REG = 'f',
    IS_LNK = 'l',
    IS_DIR = 'd',
    IS_SORT = 's',
    IS_ALL_OPT = '-'
};

#define COUNT_OPTIONS 4
#define CURRENT_DIR "."
#define RELOAD_OPTIND optind = 1
#define FIND_PATH_DIR while(getopt(argc, argv, "ldfs") != -1) { }
bool DOES_THE_FILE_EXIST(const char* path) { DIR* d; if ((d = opendir(path)) == NULL) return false; closedir(d); return true; }

void alloc_memory_and_copy(char*** arr, size_t* arr_size, size_t* capacity, const char* fullpath) {
    if (*arr_size + 1 == *capacity) {
        *capacity *= 2;
        *arr = (char**)realloc(*arr, *capacity * sizeof(char*));
    }
    int len_path = strlen(fullpath) + 1;
    (*arr)[*arr_size] = (char*)malloc(len_path);
    strncpy((*arr)[(*arr_size)++], fullpath, len_path);
}

void dirwalk(char* path, const char options[], size_t cnt_options, char*** arr, size_t* arr_size, size_t* capacity){
    if (!path) return;
    struct dirent* dir;
    struct stat sb;
    DIR* d = opendir(path);
    while((dir = readdir(d))){
        char fullpath[MAXPATHLEN];
        if (strcmp(dir->d_name, ".") == 0 || strcmp(dir->d_name, "..") == 0)
            continue;
        sprintf(fullpath, "%s/%s", path, dir->d_name);
        if (lstat(fullpath, &sb) == -1)
            continue;
        for (size_t i = 0; i < cnt_options; ++i) {
            if ((sb.st_mode & __S_IFMT) == __S_IFDIR){
                if (options[i] == IS_DIR || options[i] == IS_ALL_OPT) {
                    alloc_memory_and_copy(arr, arr_size, capacity, fullpath);
                }
                dirwalk(fullpath, options, cnt_options, arr, arr_size, capacity);
            }
            else if ((sb.st_mode & __S_IFMT) == __S_IFLNK && (options[i] == IS_LNK || options[i] == IS_ALL_OPT)) {
                alloc_memory_and_copy(arr, arr_size, capacity, fullpath);
            }
            else if ((sb.st_mode & __S_IFMT) == __S_IFREG && (options[i] == IS_REG || options[i] == IS_ALL_OPT)) {
                alloc_memory_and_copy(arr, arr_size, capacity, fullpath);
            }
        }
    }
    closedir(d);
}


int main(int argc, char* argv[]){
    setlocale(LC_COLLATE, "ru_RU.UTF-8");
    char** arr_path = NULL;
    size_t size_arr = 0, capacity = 1;
    bool FLAG_LC_COLLATE_SORT = false;
    FIND_PATH_DIR;
    char* path = NULL;
    if (optind < argc) {
        const int str_len = strlen(argv[optind]);
        path = (char*)malloc(str_len);
        strncpy(path, argv[optind], str_len);
    }
    else path = CURRENT_DIR;
    RELOAD_OPTIND;
    if (!DOES_THE_FILE_EXIST(path)) {
        printf("Check if the directory or file actually exists!");
    }else {
        bool flag_are_there_any_options = false;
        int option_ch; char options[COUNT_OPTIONS]; size_t cnt_options = 0;
        while((option_ch = getopt(argc, argv, "ldfs")) != -1){
            switch(option_ch) {
                case IS_LNK : { options[cnt_options++] = IS_LNK;  break; }
                case IS_REG : { options[cnt_options++] = IS_REG;   break; }
                case IS_DIR : { options[cnt_options++] = IS_DIR;  break; }
                case IS_SORT : { FLAG_LC_COLLATE_SORT = true; break; }
                default : break;
            }
            if (option_ch != IS_SORT)
                flag_are_there_any_options = true;
        }
        if (!flag_are_there_any_options) options[cnt_options++] = '-';
        dirwalk(path, options, cnt_options, &arr_path, &size_arr, &capacity);
    }
    if (arr_path) {
        if (FLAG_LC_COLLATE_SORT)
            qsort(arr_path, size_arr, sizeof(char*), strcoll);
        for (size_t j = 0; j < size_arr; ++j) {
            printf("%s\n", arr_path[j]);
        }
        for (size_t i = 0; i < size_arr; ++i)
            free(arr_path[i]);
        free(arr_path);
    }
    return 0;
}
