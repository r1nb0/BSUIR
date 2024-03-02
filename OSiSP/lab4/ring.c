#include "ring.h"

node_ring* constructor_node() {
    int32_t shmid = shmget(0, sizeof(node_ring), 0777);
    node_ring* buffer = shmat(shmid, NULL, 0);
    buffer->shmid_curr = shmid;
    buffer->shmid_next = shmid;
    buffer->shmid_prev = shmid;
    buffer->flag_is_busy = false;
    return buffer;
}

ring_shared_buffer* constructor_buffer() {
    int32_t shmid = shmget(0, sizeof(ring_shared_buffer), 0777);
    ring_shared_buffer* buffer = shmat(shmid, NULL, 0);
    buffer->shmid_tail = 0;
    buffer->shmid_begin = 0;
    buffer->consumed = 0;
    buffer->produced = 0;
    buffer->shmid = shmid;
    return buffer;
}

void append(ring_shared_buffer** __begin) {
    if (__begin == NULL)
        exit(-100);
    if (*__begin == NULL) {
        *__begin = constructor_buffer();
    }
    node_ring* __buffer = constructor_node();
    if ((*__begin)->shmid_begin == 0) {
        (*__begin)->shmid_begin = __buffer->shmid_curr;
        (*__begin)->shmid_tail = __buffer->shmid_curr;
        return;
    }
    node_ring* __curr = shmat((*__begin)->shmid_begin, NULL, 0);
    if (__curr->shmid_curr == __curr->shmid_next) {
        __buffer->shmid_next = __buffer->shmid_prev = __curr->shmid_curr;
        __curr->shmid_next = __curr->shmid_prev = __buffer->shmid_curr;
        return;
    }
    node_ring* __prev = shmat(__curr->shmid_prev, NULL, 0);
    __buffer->shmid_next = __curr->shmid_curr;
    __buffer->shmid_prev = __prev->shmid_curr;
    __prev->shmid_next = __buffer->shmid_curr;
    __curr->shmid_prev = __buffer->shmid_curr;
}

void add_message(ring_shared_buffer* __ring, const char* __message) {
    if (__ring == NULL) {
        printf("The ring is empty.\n");
        return;
    }
    if (__ring->shmid_begin == 0) {
        printf("There are 0 places in the ring.\n");
        return;
    }
    node_ring* __curr = shmat(__ring->shmid_tail, NULL, 0);
    if (__curr->flag_is_busy == true) {
        printf("No free places.\n");
        return;
    }
    strncpy(__curr->message, __message, LEN_MESSAGE * sizeof(char));
    __curr->flag_is_busy = true;
    __ring->shmid_tail = __curr->shmid_next;
    __ring->produced++;
}

char* extract_message(ring_shared_buffer* __ring) {
    if (__ring == NULL) {
        printf("The ring is empty.\n");
        return NULL;
    }
    if (__ring->shmid_begin == 0) {
        printf("There are 0 places in the ring.\n");
        return NULL;
    }
    node_ring* __curr = shmat(__ring->shmid_begin, NULL, 0);
    if (__curr->flag_is_busy == false) {
        printf("No messages to retrieve.\n");
        return NULL;
    }
    __curr->flag_is_busy = false;
    char* __message = calloc(LEN_MESSAGE, sizeof(char));
    strncpy(__message, __curr->message,  LEN_MESSAGE * sizeof(char));
    __ring->shmid_begin = __curr->shmid_next;
    __ring->consumed++;
    return __message;
}