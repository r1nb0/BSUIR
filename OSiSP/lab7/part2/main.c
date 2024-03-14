#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <stdbool.h>
#include <fcntl.h>
#include <unistd.h>

#define MAX_LEN_STRUCT_RECORD 80
#define COUNT_RECORDS 10

typedef struct record_s {
    char name[MAX_LEN_STRUCT_RECORD];
    char address[MAX_LEN_STRUCT_RECORD];
    u_int8_t semester;
}record_t;

int descriptor = 0;
bool FLAG_EDIT = false;


bool is_equal(const record_t* __first, const record_t* __second);
bool record_copy(record_t* __dest, const record_t* __source);
void all_records();
bool get_record(size_t num_rec, record_t* __record);
void modify(size_t num_rec, record_t* __record, record_t* __record_save);
bool put_record(record_t* __record_new, const record_t* __record, const record_t* __record_save, size_t num_rec);

int main(int argc, char* argv[])
{
    bool flag_continue = true;
    record_t REC;
    record_t REC_SAVE;
    record_t REC_NEW;
    size_t NUM_REC = 0;

    descriptor = open("lab7part2.bin", O_RDWR);

    if (descriptor == -1) {
        printf("ERROR: Wrong file name.\n");
        exit(0);
    }

    do {
        printf("1.Display all records.\n");
        printf("2.Modify record.\n");
        printf("3.Get num_record.\n");
        printf("4.Put last modified record.\n");
        char ch = getchar();
        switch(ch) {
            case '1': {
                all_records();
                break;
            }
            case '2' : {
                printf("Enter the num of record: ");
                scanf("%lu", &NUM_REC);
                modify(NUM_REC, &REC, &REC_SAVE);
                break;
            }
            case '3' : {
                record_t record;
                size_t num;
                printf("Enter the num of record: ");
                scanf("%lu", &num);
                get_record(num, &record);
                printf("%lu. Name: %s, Address: %s, Num of semester: %hhu\n", num, record.name, record.address, record.semester);
                break;
            }
            case '4' : {
                if (!FLAG_EDIT) {
                    printf("No entry has been changed!\n");
                }else {
                    bool flag_put = put_record(&REC_NEW, &REC, &REC_SAVE, NUM_REC);
                    if (flag_put == false) {
                        printf("The data has been changed by someone, please repeat the editing operation again.\n");
                        modify(NUM_REC, &REC, &REC_SAVE);
                    }
                }
                break;
            }
            default : { flag_continue = false; break; }
        }
        getchar();
    }while(flag_continue);

    int status_close = close(descriptor);
    if (status_close == -1) {
        printf("Unable to close file.\n");
        exit(0);
    }

    close(descriptor);

    return 0;
}

bool is_equal(const record_t* __first, const record_t* __second) {
    if (__first == NULL || __second == NULL)
        exit(-100);
    if (strcmp(__first->name, __second->name) == 0 &&
        strcmp(__first->address, __second->address) == 0 &&
        __first->semester == __second->semester)
        return true;
    return false;
}

bool record_copy(record_t* __dest, const record_t* __source) {
    if (__dest == NULL || __source == NULL)
        exit(-100);
    strncpy(__dest->name, __source->name, MAX_LEN_STRUCT_RECORD);
    strncpy(__dest->address, __source->address, MAX_LEN_STRUCT_RECORD);
    __dest->semester = __source->semester;
    return true;
}


void all_records() {
    struct flock parameters;
    parameters.l_type = F_RDLCK;
    parameters.l_whence = SEEK_SET;
    parameters.l_start = 0;
    parameters.l_len = 0;
    fcntl(descriptor, F_SETLK, &parameters);
    record_t buffer;
    for (size_t i = 0; i < COUNT_RECORDS; ++i) {
        read(descriptor, &buffer, sizeof(record_t));
        printf("%lu. Name: %s, Address: %s, Num of semester: %hhu\n", i+1, buffer.name, buffer.address, buffer.semester);
    }
    parameters.l_type = F_UNLCK;
    fcntl(descriptor, F_SETLK, &parameters);
    lseek(descriptor, 0, SEEK_SET);
}

bool get_record(size_t num_rec, record_t* __record) {
    if (num_rec > COUNT_RECORDS || num_rec == 0) {
        return false;
    }
    struct flock parameters;
    parameters.l_type = F_RDLCK;
    parameters.l_whence = SEEK_SET;
    parameters.l_start = (num_rec - 1) * sizeof(record_t);
    parameters.l_len = sizeof(record_t);
    fcntl(descriptor, F_SETLK, &parameters);
    lseek(descriptor, (num_rec - 1) * sizeof(record_t), SEEK_SET);
    read(descriptor, __record, sizeof(record_t));
    parameters.l_type = F_UNLCK;
    fcntl(descriptor, F_SETLK, &parameters);
    lseek(descriptor, 0, SEEK_SET);
    return true;
}

void modify(size_t num_rec, record_t* __record, record_t* __record_save) {
    bool flag_exist = get_record(num_rec, __record);
    if (!flag_exist)
        return;
    record_copy(__record_save, __record);
    bool flag_continue = true;
    getchar();
    do {
        printf("1.Edit name.\n");
        printf("2.Edit address.\n");
        printf("3.Edit num of semester.\n");
        printf("4.Display edit record.\n");
        char ch = getchar();
        switch(ch) {
            case '1': {
                FLAG_EDIT = true;
                break;
            }
            case '2': {
                FLAG_EDIT = true;
                break;
            }
            case '3': {
                FLAG_EDIT = true;
                u_int8_t sem;
                printf("Enter the num of semester: ");
                scanf("%hhu", &sem);
                __record->semester = sem;
                break;
            }
            case '4' : {
                printf("%lu. Name: %s, Address: %s, Num of semester: %hhu\n", num_rec, __record->name, __record->address, __record->semester);
                break;
            }
            default: {
                flag_continue = false;
                break;
            }
        }
        getchar();
    }while(flag_continue);
}

bool put_record(record_t* __record_new, const record_t* __record, const record_t* __record_save, size_t num_rec) {
    struct flock parameters;
    parameters.l_type = F_WRLCK;
    parameters.l_whence = SEEK_SET;
    parameters.l_start = (num_rec - 1) * sizeof(record_t);
    parameters.l_len = sizeof(record_t);
    fcntl(descriptor, F_SETLK, &parameters);
    parameters.l_type = F_UNLCK;
    get_record(num_rec, __record_new);
    if (!is_equal(__record_save, __record_new)) {
        fcntl(descriptor, F_SETLK, &parameters);
        return false;
    }
    lseek(descriptor, (num_rec - 1) * sizeof(record_t), SEEK_SET);
    write(descriptor, __record, sizeof(record_t));
    fcntl(descriptor, F_SETLK, &parameters);
    lseek(descriptor, 0, SEEK_SET);
    FLAG_EDIT = false;
    return true;
}


// descriptor = open("lab7part2.bin", O_RDWR);
    //
    // record_t record1;
    // strncpy(record1.name, "NAME_1", MAX_LEN_STRUCT_RECORD);
    // strncpy(record1.address, "ADDRESS_1", MAX_LEN_STRUCT_RECORD);
    // record1.semester = 1;
    // record_t record2;
    // strncpy(record2.name, "NAME_2", MAX_LEN_STRUCT_RECORD);
    // strncpy(record2.address, "ADDRESS_2", MAX_LEN_STRUCT_RECORD);
    // record2.semester = 2;
    // record_t record3;
    // strncpy(record3.name, "NAME_3", MAX_LEN_STRUCT_RECORD);
    // strncpy(record3.address, "ADDRESS_3", MAX_LEN_STRUCT_RECORD);
    // record3.semester = 3;
    // record_t record4;
    // strncpy(record4.name, "NAME_4", MAX_LEN_STRUCT_RECORD);
    // strncpy(record4.address, "ADDRESS_4", MAX_LEN_STRUCT_RECORD);
    // record4.semester = 4;
    // record_t record5;
    // strncpy(record5.name, "NAME_5", MAX_LEN_STRUCT_RECORD);
    // strncpy(record5.address, "ADDRESS_5", MAX_LEN_STRUCT_RECORD);
    // record5.semester = 5;
    // record_t record6;
    // strncpy(record6.name, "NAME_6", MAX_LEN_STRUCT_RECORD);
    // strncpy(record6.address, "ADDRESS_6", MAX_LEN_STRUCT_RECORD);
    // record6.semester = 6;
    // record_t record7;
    // strncpy(record7.name, "NAME_7", MAX_LEN_STRUCT_RECORD);
    // strncpy(record7.address, "ADDRESS_7", MAX_LEN_STRUCT_RECORD);
    // record7.semester = 7;
    // record_t record8;
    // strncpy(record8.name, "NAME_8", MAX_LEN_STRUCT_RECORD);
    // strncpy(record8.address, "ADDRESS_8", MAX_LEN_STRUCT_RECORD);
    // record8.semester = 8;
    // record_t record9;
    // strncpy(record9.name, "NAME_9", MAX_LEN_STRUCT_RECORD);
    // strncpy(record9.address, "ADDRESS_9", MAX_LEN_STRUCT_RECORD);
    // record9.semester = 9;
    // record_t record10;
    // strncpy(record10.name, "NAME_10", MAX_LEN_STRUCT_RECORD);
    // strncpy(record10.address, "ADDRESS_10", MAX_LEN_STRUCT_RECORD);
    // record10.semester = 10;
    //
    // write(descriptor, &record1, sizeof(record_t));
    // write(descriptor, &record2, sizeof(record_t));
    // write(descriptor, &record3, sizeof(record_t));
    // write(descriptor, &record4, sizeof(record_t));
    // write(descriptor, &record5, sizeof(record_t));
    // write(descriptor, &record6, sizeof(record_t));
    // write(descriptor, &record7, sizeof(record_t));
    // write(descriptor, &record8, sizeof(record_t));
    // write(descriptor, &record9, sizeof(record_t));
    // write(descriptor, &record10, sizeof(record_t));