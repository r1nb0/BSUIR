#include <sys/types.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct index_s {
    double time_mark;
    u_int64_t recno;
} index_record;

enum ARGUMENTS {
    MEMSIZE = 1,
    BLOCKS = 2,
    THREADS = 3,
    FILENAME = 4
};

pthread_mutex_t mutex;
pthread_barrier_t barrier;
size_t threads = 0;
size_t memsize = 0;
size_t fullsize = 0;
size_t repeats = 0;
size_t blocks = 0;
size_t size_block = 0;
size_t cnt_records = 0;
int fd_file = 0;
bool *state_map = NULL;
index_record *current_memsize_block = NULL;
_Thread_local size_t j_thread = 0;

typedef struct pthread_index {
    size_t initial_index;
    size_t current_index;
} pthread_index;

int compare(const void *a, const void *b) {
    return (int)((index_record *) a)->time_mark - (int)((index_record *) b)->time_mark;
}

void merge(index_record *buffer, size_t _cnt) {
    index_record *left = (index_record *)malloc(_cnt * sizeof(index_record));
    index_record *right = (index_record *)malloc(_cnt * sizeof(index_record));
    memcpy(left, buffer, _cnt * sizeof(index_record));
    memcpy(right, buffer + _cnt, _cnt * sizeof(index_record));
    size_t i = 0;
    size_t j = 0;

    while (i < _cnt && j < _cnt) {
        if (left[i].time_mark > right[j].time_mark) {
            buffer[i + j].time_mark = right[j].time_mark;
            buffer[i + j].recno = right[j].recno;
            j++;
        } else {
            buffer[i + j].time_mark = left[i].time_mark;
            buffer[i + j].recno = left[i].recno;
            i++;
        }
    }

    while (i < _cnt) {
        buffer[i + j].time_mark = left[i].time_mark;
        buffer[i + j].recno = left[i].recno;
        i++;
    }

    while (j < _cnt) {
        buffer[i + j].time_mark = right[j].time_mark;
        buffer[i + j].recno = right[j].recno;
        j++;
    }

    free(left);
    free(right);
}

void memsize_merge(void *_index) {
    pthread_index* index = (pthread_index*)_index;
    while(true) {
        pthread_barrier_wait(&barrier);
        index->current_index = index->initial_index;
        if (index->initial_index == 0) {
            printf("curr\n");
            printf("%lu\n", index->initial_index);
            blocks /= 2;
            cnt_records *= 2;
            state_map = (bool*)realloc(state_map, blocks * sizeof(bool));
            for (size_t i = 0; i < blocks; i++) {
                state_map[i] = false;
            }
        }
        pthread_barrier_wait(&barrier);
        if (blocks == 1) {
            pthread_barrier_wait(&barrier);
            return;
        }
        pthread_mutex_lock(&mutex);
        for (size_t i = 0; i < blocks; i++) {
            if (state_map[i] == false) {
                state_map[i] = true;
                index->current_index = i;
                pthread_mutex_unlock(&mutex);
                merge(current_memsize_block + cnt_records * index->current_index, cnt_records / 2);
                pthread_mutex_lock(&mutex);
            }
        }
        pthread_mutex_unlock(&mutex);
    }
}

void memsize_sort(void *_index) {
    pthread_index *index = (pthread_index *)(_index);
    pthread_barrier_wait(&barrier);
    qsort(current_memsize_block + cnt_records * index->current_index, cnt_records, sizeof(index_record), compare);
    pthread_mutex_lock(&mutex);
    for (size_t i = threads + 1; i < blocks; i++) {
        if (state_map[i] == false) {
            state_map[i] = true;
            index->current_index = i;
            pthread_mutex_unlock(&mutex);
            qsort(current_memsize_block + cnt_records * index->current_index, cnt_records, sizeof(index_record), compare);
            pthread_mutex_lock(&mutex);
        }
    }
    pthread_mutex_unlock(&mutex);
    if (index->initial_index == 0) {
        for (size_t i = 0; i < blocks; i++) {
            state_map[i] = false;
        }
    }
    memsize_merge(index);
}

void mmap_file(const char *_filename, size_t _i) {
    pthread_index main_pthread_ind = {0, 0};
    current_memsize_block = (index_record *)mmap(NULL, memsize, PROT_READ | PROT_WRITE, MAP_SHARED, fd_file, memsize * _i);
    memsize_sort(&main_pthread_ind);
}


void initial_map(size_t _blocks) {
    if (state_map != NULL) {
        state_map = (bool*)realloc(state_map, _blocks * sizeof(bool));
    } else {
        state_map = (bool*)calloc(_blocks, sizeof(bool));
    }
    for (size_t i = 0; i < _blocks; i++) {
        state_map[i] = false;
    }
}

void pthread_func(void* _index) {
    pthread_index *index = (pthread_index *)(_index);
    for (j_thread = 0; j_thread < repeats; j_thread++) {
        memsize_sort(_index);
    }
}


int main(int argc, char *argv[]) {
    //check errors

    memsize = atoi(argv[MEMSIZE]);
    blocks = atoi(argv[BLOCKS]);
    threads = atoi(argv[THREADS]) - 1;
    const char *filename = argv[FILENAME];
    size_block = memsize / blocks;
    cnt_records = size_block / sizeof(index_record);


    FILE *file = fopen(filename, "rb+");
    if (file == NULL) {
        fprintf(stderr, "Error file.\n");
        exit(EXIT_FAILURE);
    }
    fseek(file, 0, SEEK_END);
    fullsize = ftell(file);
    repeats = fullsize / memsize;
    fseek(file, 0, SEEK_SET);
    fd_file = fileno(file);


    pthread_t pthrarray[threads];

    pthread_mutex_init(&mutex, NULL);
    pthread_barrier_init(&barrier, NULL, threads + 1);

    pthread_index indexes[threads];

    for (size_t i = 0; i < threads; ++i) {
        indexes[i].current_index = indexes[i].initial_index = i + 1;
    }

    for (size_t i = 0; i < threads; ++i) {
        pthread_create(&pthrarray[i], NULL, (void *(*)(void *))pthread_func, &indexes[i]);
    }

    size_t save_block = blocks;
    size_t save_cnt = cnt_records;
    for (size_t i = 0; i < repeats; i++) {
        printf("%lu\n", i);
        initial_map(save_block);
        mmap_file(filename, i);
        merge(current_memsize_block, cnt_records / 2);
        munmap(current_memsize_block, memsize);
        blocks = save_block;
        cnt_records = save_cnt;
        fseek(file, 0, SEEK_SET);
    }
    
    for (size_t i = 0; i < threads; ++i) {
        pthread_join(pthrarray[i], NULL);
    }
    
    fclose(file);
    pthread_mutex_destroy(&mutex);
    pthread_barrier_destroy(&barrier);

    free(state_map);

    return 0;
}
