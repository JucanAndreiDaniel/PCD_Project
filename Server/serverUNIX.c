#include <errno.h>
#include <openssl/aes.h>
#include <openssl/md5.h>
#include <openssl/sha.h>
#include <pthread.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>
// #include <pthread.h>



int unix_socket(const char *filename)
{
    struct sockaddr_un name;
    int sock; /* UNIX Socket descriptor */
    size_t size;

    /* Create the socket. */
    sock = socket(PF_LOCAL, SOCK_DGRAM, 0);
    if (sock < 0)
    {
        perror("unix-common: socket error");
        pthread_exit(NULL);
    }

    name.sun_family = AF_LOCAL; /* Set ADDRESS Family */
    strncpy(name.sun_path, filename, sizeof(name.sun_path));
    /* Create SOCKET Path info */
    name.sun_path[sizeof(name.sun_path) - 1] = '\0';

    size = (offsetof(struct sockaddr_un, sun_path) + strlen(name.sun_path) + 1);
    /* You can use size = SUN_LEN (&name) ; instead */

    /* Now BIND the socket */
    if (bind(sock, (struct sockaddr *)&name, size) < 0)
    {
        perror("bind");
        pthread_exit(NULL);
    }

    /* And RETURN success :) */
    return sock;
}

void *unix_main(void *args)
{
    char *socket = (char *)args;
    int sock = unix_socket(socket);
    struct sockaddr_un client;
    socklen_t client_len = sizeof(client);
    char buffer[1024];
    int n;
    while (1)
    {
        break;
    }
    pthread_exit(NULL);
}