#include <sys/types.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <sys/mman.h>
#include <pthread.h>
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
bool *state_map;
index_record *current_memsize_block = NULL;

typedef struct pthread_index {
    size_t initial_index;
    size_t current_index;
    bool is_rec;
} pthread_index;

int compare(const void *a, const void *b) {
    return ((index_record *) a)->time_mark - ((index_record *) b)->time_mark;
}

void merge(index_record *buffer, size_t __cnt) {
    index_record *left = (index_record *)calloc(__cnt, sizeof(index_record));
    index_record *right = (index_record *)calloc(__cnt, sizeof(index_record));
    memcpy(left, buffer, __cnt * sizeof(index_record));
    memcpy(right, buffer + __cnt, __cnt * sizeof(index_record));
    size_t i = 0;
    size_t j = 0;

    while (i < __cnt && j < __cnt) {
        if (left[i].time_mark > right[j].time_mark) {
            buffer[i + j].time_mark = right[j].time_mark;
            buffer[i + j].recno = right[j++].recno;
        } else {
            buffer[i + j].time_mark = left[i].time_mark;
            buffer[i + j].recno = left[i++].recno;
        }
    }

    while (i < __cnt) {
        buffer[i + j].time_mark = left[i].time_mark;
        buffer[i + j].recno = left[i++].recno;
    }

    while (j < __cnt) {
        buffer[i + j].time_mark = right[j].time_mark;
        buffer[i + j].recno = right[j++].recno;
    }

    free(left);
    free(right);
}


void memsize_merge(void *__index) {
    pthread_index* index = (pthread_index*)__index;
    while(blocks != 1) {
        pthread_barrier_wait(&barrier);
        index->current_index = index->initial_index;
        if (index->initial_index == 0) {
            blocks /= 2;
            cnt_records *= 2;
            state_map = (bool*)realloc(state_map, blocks * sizeof(bool));
            for (size_t i = 0; i < blocks; i++) {
                state_map[i] = false;
            }
        }
        pthread_barrier_wait(&barrier);
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
    if (index->initial_index == 0) {
        merge(current_memsize_block, cnt_records / 2);
        munmap(current_memsize_block, memsize);
    }
}

void memsize_sort(void *__index) {
    pthread_index *index = (pthread_index *)(__index);
    if (index->is_rec == false) {
        pthread_barrier_wait(&barrier);
    }
    printf("Sorted form index: %lu\n", index->current_index);
    qsort(current_memsize_block + cnt_records * index->current_index, cnt_records, sizeof(index_record), compare);

    pthread_mutex_lock(&mutex);
    for (size_t i = threads + 1; i < blocks; i++) {
        if (state_map[i] == false) {
            state_map[i] = true;
            index->is_rec = true;
            index->current_index = i;
            pthread_mutex_unlock(&mutex);
            memsize_sort(index);
            return;
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

void mmap_file(const char *__filename) {
    pthread_index main_pthread_ind = {0, 0};
    FILE *file = fopen(__filename, "rb+");
    if (file == NULL) {
        fprintf(stderr, "Error file.\n");
        exit(EXIT_FAILURE);
    }
    fseek(file, 0, SEEK_SET);
    int fd = fileno(file);
    current_memsize_block = (index_record *)mmap(NULL, memsize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    fclose(file);
    memsize_sort(&main_pthread_ind);
}


int main(int argc, char *argv[]) {


    //check errors

    memsize = atoi(argv[MEMSIZE]);
    blocks = atoi(argv[BLOCKS]);
    threads = atoi(argv[THREADS]) - 1;
    const char *filename = argv[FILENAME];
    size_block = memsize / blocks;
    cnt_records = size_block / sizeof(index_record);

    pthread_t pthrarray[threads];

    pthread_mutex_init(&mutex, NULL);
    pthread_barrier_init(&barrier, NULL, threads + 1);


    printf("Threads : %lu\n", threads + 1);
    printf("Memsize: %lu\n", memsize);
    printf("Blocks: %lu\n", blocks);
    printf("Size_blocks: %lu\n", size_block);
    printf("Cnt records: %lu\n", cnt_records);


    state_map = (bool*)calloc(blocks, sizeof(bool));

    for (size_t i = 0; i < blocks; ++i) {
        state_map[i] = false;
    }

    pthread_index indexes[threads];

    for (size_t i = 0; i < threads; ++i) {
        indexes[i].current_index = indexes[i].initial_index = i + 1;
        indexes[i].is_rec = false;
    }

    for (size_t i = 0; i < threads; ++i) {
        pthread_create(&pthrarray[i], NULL, memsize_sort, &indexes[i]);
    }

    FILE *file = fopen(filename, "rb+");
    if (file == NULL) {
        fprintf(stderr, "Error file.\n");
        exit(EXIT_FAILURE);
    }
    fseek(file, 0, SEEK_END);
    fullsize = ftell(file);
    fseek(file, 0, SEEK_SET);

    repeats = fullsize / memsize;

    printf("Full size file: %lu\n", fullsize);
    printf("Repeats: %lu\n", repeats);

    mmap_file(filename);

    for (size_t i = 0; i < threads; ++i) {
        pthread_join(pthrarray[i], NULL);
    }

    pthread_mutex_destroy(&mutex);
    pthread_barrier_destroy(&barrier);

    free(state_map);

    return 0;
}
