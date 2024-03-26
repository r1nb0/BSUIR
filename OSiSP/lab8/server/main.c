#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <time.h>

#define MAX_LEN_BUFFER 1024
#define LEN_DATA 40

#define ECHO_QUERY "ECHO"
#define QUIT_QUERY "QUIT"
#define INFO_QUERY "INFO"
#define LIST_QUERY "LIST"
#define CD_QUERY "CD"
#define UNKNOWN_QUERY "UNKNOWN"

#define START_MESSAGE "myclient server.AF_INET\n"
#define EXIT_MESSAGE "The session has ended. Goodbye.\n"
#define WRONG_PARAMETER "You entered an incorrect parameter.\nList of available parameters: [CD, ECHO, LIST, QUIT, INFO].\n"

char QUERY[MAX_LEN_BUFFER];
char BUFFER[MAX_LEN_BUFFER];

enum LEVEL_LOGGER {
    __FATAL = 0,
    __ERROR = 1,
    __WARN = 2,
    __INFO = 3,
    __DEBUG = 4,
    __TRACE = 5
};

void logging_console(int fd_client, const char* __command, const char* __result, unsigned int __loger_level) {
    const time_t date_logging = time(NULL);
    struct tm* u = localtime(&date_logging);
    char str_date[LEN_DATA];
    strftime(str_date, 40, "%d.%m.%Y %H:%M:%S ", u);
    switch(__loger_level) {
        case __FATAL : {
            printf("\033[1;31m%sFATAL\033[0m %s from [%d] with arguments = %s",
                str_date, __command, fd_client, __result);
            break;
        }
        case __ERROR : {
            printf("\033[1;31m%sERROR\033[0m %s from [%d] with arguments = %s",
                str_date, __command, fd_client, __result);
            break;
        }
        case __WARN : {
            printf("\033[1;33m%sWARN\033[0m %s from [%d] with arguments = %s",
                str_date, __command, fd_client, __result);
            break;
        }
        case __INFO : {
            printf("\033[1;32m%sINFO\033[0m %s from [%d] with arguments = %s",
                str_date, __command, fd_client, __result);
            break;
        }
        case __DEBUG : {
            printf("\033[1;34m%sDEBUG\033[0m %s from [%d] with arguments = %s",
                str_date, __command, fd_client, __result);
            break;
        }
        case __TRACE : {
            printf("\033[1;36m%sTRACE\033[0m %s from [%d] with arguments = %s",
                str_date, __command, fd_client, __result);
            break;
        }
        default: break;
    }
}


int main(int argc, char* argv[]) {

    // if (argc != 2) {
    //     fprintf(stderr, "Usage: %s port\n", argv[0]);
    //     exit(EXIT_FAILURE);
    // }

    struct addrinfo hints = {0};
    struct addrinfo* result = NULL;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    hints.ai_protocol = 0;
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;

    int soc = getaddrinfo(NULL, "8080",  &hints, &result);

    if (soc != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(soc));
        exit(EXIT_FAILURE);
    }

    int socfd = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    bind(socfd, result->ai_addr, result->ai_addrlen);

    printf("myserver %s\n", /*argv[1]*/ "8080");
    printf("Ready.\n");

    listen(socfd, SOMAXCONN);
    int fd_client = accept(socfd, result->ai_addr, &result->ai_addrlen);
    write(fd_client, START_MESSAGE, strlen(START_MESSAGE));


    while(true) {
        ssize_t bytes_read = recv(fd_client, QUERY, MAX_LEN_BUFFER, 0);
        if (bytes_read > 0) {
            if (strncasecmp(QUERY, ECHO_QUERY, strlen(ECHO_QUERY)) == 0) {
                logging_console(fd_client, ECHO_QUERY, QUERY, __DEBUG);
                write(fd_client, QUERY, strlen(QUERY));
            }

            else if (strncasecmp(QUERY, INFO_QUERY, strlen(INFO_QUERY)) == 0) {
                logging_console(fd_client,INFO_QUERY, QUERY, __DEBUG);
            }

            else if (strncasecmp(QUERY, CD_QUERY, strlen(CD_QUERY)) == 0) {
                logging_console(fd_client,CD_QUERY, QUERY, __DEBUG);
            }

            else if (strncasecmp(QUERY, LIST_QUERY, strlen(LIST_QUERY)) == 0) {
                logging_console(fd_client,LIST_QUERY, QUERY, __DEBUG);
            }

            else if (strncasecmp(QUERY, QUIT_QUERY, strlen(QUIT_QUERY)) == 0) {
                logging_console(fd_client,QUIT_QUERY, QUERY, __DEBUG);
                write(fd_client, EXIT_MESSAGE, strlen(EXIT_MESSAGE));
            }
            else {
                logging_console(fd_client,UNKNOWN_QUERY, QUERY, __ERROR);
                write(fd_client, WRONG_PARAMETER, strlen(WRONG_PARAMETER));
            }
        } else if (bytes_read == 0) {
            printf("Client is closed connection\n");
            break;
        } else {
            fprintf(stderr, "Error of reading data.\n");
        }
    }
    close(fd_client);
    close(socfd);
    return 0;
}
