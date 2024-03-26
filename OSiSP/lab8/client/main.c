#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdbool.h>

#define MAX_LEN_BUFFER 1024
#define QUIT_QUERY "QUIT"

bool FLAG_CONTINUE = true;
char BUFFER[MAX_LEN_BUFFER];
char CORRECT_BUFFER[MAX_LEN_BUFFER];
char QUERY[MAX_LEN_BUFFER];

//strtok for correct string

int main(int argc, char* argv[])
{
    struct addrinfo hints = {0};
    struct addrinfo* result = NULL;

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = 0;
    hints.ai_flags = 0;

    int s = getaddrinfo(NULL, "8080", &hints, &result);

    if (s != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        exit(EXIT_FAILURE);
    }

    int fd_client = socket(hints.ai_family, hints.ai_socktype, hints.ai_addrlen);
    int flag_connect = connect(fd_client, result->ai_addr, result->ai_addrlen);

    if (flag_connect == -1) {
        printf("Server is close\n");
        exit(EXIT_FAILURE);
    }

    read(fd_client, BUFFER, MAX_LEN_BUFFER);
    printf("%s", BUFFER);


    while(FLAG_CONTINUE) {
        printf("> ");
        fgets(QUERY,MAX_LEN_BUFFER, stdin);
        write(fd_client, QUERY, MAX_LEN_BUFFER);
        if (strncasecmp(QUERY, QUIT_QUERY, strlen(QUIT_QUERY)) == 0) {
            FLAG_CONTINUE = false;
        }
        ssize_t bytes_read = recv(fd_client, QUERY, MAX_LEN_BUFFER, 0);
        QUERY[bytes_read] = '\0';
        if (bytes_read > 0) {
            fputs(QUERY, stdout);
        } else {
            fprintf(stderr, "The server dropped the connection");
            exit(EXIT_FAILURE);
        }
    }

    close(fd_client);
    return 0;
}
