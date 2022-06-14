#include <stddef.h>
#include <errno.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/un.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <openssl/sha.h>
#include <openssl/md5.h>
#include <openssl/aes.h>
// #include <pthread.h>

#include "../shared/types.h"
#include "../shared/Utils.h"

#define GENERIC_CONTEXT_SIZE \
    (2 * 1024) // 2048 bytes (2KB) should be enought for any context

void call_sum_init(algo_type_t algo, void *CTX)
{
    switch (algo)
    {
    case SHA256_T:
        (void)SHA256_Init((SHA256_CTX *)CTX); // need to check error
        break;
    case SHA512_T:
        (void)SHA512_Init((SHA512_CTX *)CTX); // need to check error
        break;
    case MD5_T:
        (void)MD5_Init((MD5_CTX *)CTX); // need to check error
        break;
    case SHA1_T:
        (void)SHA1_Init((SHA_CTX *)CTX); // need to check error
        break;
    case SHA224_T:
        (void)SHA224_Init((SHA256_CTX *)CTX); // need to check error
        break;
    case SHA384_T:
        (void)SHA384_Init((SHA512_CTX *)CTX); // need to check error
        break;
    default:
        break;
    }
}

void call_sum_update(algo_type_t algo, void *CTX, uint8_t *data,
                     long data_len)
{
    switch (algo)
    {
    case SHA256_T:
        (void)SHA256_Update((SHA256_CTX *)CTX, data, data_len);
        break;
    case SHA512_T:
        (void)SHA512_Update((SHA512_CTX *)CTX, data, data_len);
        break;
    case MD5_T:
        (void)MD5_Update((MD5_CTX *)CTX, data, data_len);
        break;
    case SHA1_T:
        (void)SHA1_Update((SHA_CTX *)CTX, data, data_len);
        break;
    case SHA224_T:
        (void)SHA224_Update((SHA256_CTX *)CTX, data, data_len);
        break;
    case SHA384_T:
        (void)SHA384_Update((SHA512_CTX *)CTX, data, data_len);
        break;
    default:
        DBG_PRINT("default for now\n");
        break;
    }
}

void call_sum_finale(algo_type_t algo, void *CTX, uint8_t *digest)
{
    switch (algo)
    {
    case SHA256_T:
        (void)SHA256_Final(digest, (SHA256_CTX *)CTX);
        break;
    case SHA512_T:
        (void)SHA512_Final(digest, (SHA512_CTX *)CTX);
        break;
    case MD5_T:
        (void)MD5_Final(digest, (MD5_CTX *)CTX);
        break;
    case SHA1_T:
        (void)SHA1_Final(digest, (SHA_CTX *)CTX);
        break;
    case SHA224_T:
        (void)SHA224_Final(digest, (SHA256_CTX *)CTX);
        break;
    case SHA384_T:
        (void)SHA384_Final(digest, (SHA512_CTX *)CTX);
        break;
    default:
        DBG_PRINT("default for now\n");
        break;
    }
}

char *checksum_file(algo_type_t option, char *file_data, long file_size, char *file_name)
{
    void *ctx = malloc(GENERIC_CONTEXT_SIZE);

    if (NULL == ctx)
    {
        return;
    }
    call_sum_init(option, ctx);
    call_sum_update(option, (void *)ctx, file_data, file_size);
    uint8_t digest[digest_size_list[(uint8_t)option]];
    call_sum_finale(option, (void *)ctx, digest);
    // pthread_mutex_lock(&mtx);
    // FILE *fp;
    // fp = fopen(file_name, "a"); // open file to append it

    // if (NULL != fp)
    // {
    //     fprintf(fp, "%d;", (int)option);

    //     fflush(fp);

    //     for (int i = 0; i < digest_size_list[(uint8_t)option]; i++)
    //     {
    //         fprintf(fp, "%02x", digest[i]);

    //         fflush(fp);
    //     }

    //     fprintf(fp, ";%zu\n", file_size);

    //     fflush(fp);

    //     fclose(fp);
    // }

    // pthread_mutex_unlock(&mtx);
    if (NULL != ctx)
    {
        free(ctx);
    }
    char finalisimo[sizeof(digest) * 2 + 1];
    for (int i = 0, j = 0; i < digest_size_list[(uint8_t)option]; i++, j += 2)
        sprintf(finalisimo + j, "%02x", digest[i]);
    finalisimo[sizeof(digest) * 2] = 0;

    return strdup(finalisimo);
}

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
    if (sock)
    {
        // listen for connections
        int client_sock;
        struct sockaddr_un client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        while (1)
        {
            client_sock = accept(sock, (struct sockaddr *)&client_addr, &client_addr_len);
            if (client_sock < 0)
            {
                perror("accept");
                pthread_exit(NULL);
            }
            // receive data and print it
            char buf[1024];
            int len = recv(client_sock, buf, sizeof(buf), 0);
            if (len < 0)
            {
                perror("recv");
                pthread_exit(NULL);
            }
            buf[len] = '\0';
            printf("%s\n", buf);

            if (strcmp(buf, "exit") == 0)
            {
                close(client_sock);
                break;
            }
            if (strcmp(buf, "checksum") == 0)
            {
                // read from socket the algorithm
                char algo[1024];
                len = recv(client_sock, algo, sizeof(algo), 0);
                if (len < 0)
                {
                    perror("recv");
                    pthread_exit(NULL);
                }
                algo[len] = '\0';
                // get the filename
                char filename[1024];
                len = recv(client_sock, filename, sizeof(filename), 0);
                if (len < 0)
                {
                    perror("recv");
                    pthread_exit(NULL);
                }
                filename[len] = '\0';
                // read the file
                FILE *fp = fopen(filename, "r");
                if (NULL == fp)
                {
                    perror("fopen");
                    pthread_exit(NULL);
                }
                // get the file size
                fseek(fp, 0, SEEK_END);
                long file_size = ftell(fp);
                fseek(fp, 0, SEEK_SET);
                // calculate the checksum and send it
                char *checksum = checksum_file((algo_type_t)(atoi(algo)), (char *)fp, file_size, filename);
                len = send(client_sock, checksum, strlen(checksum), 0);
                if (len < 0)
                {
                    perror("send");
                    pthread_exit(NULL);
                }
                free(checksum);
                fclose(fp);
            }
            if (strcmp(buf, "verify_checksum") == 0)
            {
                char algo[1024];
                len = recv(client_sock, algo, sizeof(algo), 0);
                if (len < 0)
                {
                    perror("recv");
                    pthread_exit(NULL);
                }
                algo[len] = '\0';

                // read the checksum from the socket
                char checksum_client[1024];
                len = recv(client_sock, checksum_client, sizeof(checksum_client), 0);
                if (len < 0)
                {
                    perror("recv");
                    pthread_exit(NULL);
                }
                checksum_client[len] = '\0';

                // get the filename
                char filename[1024];
                len = recv(client_sock, filename, sizeof(filename), 0);
                if (len < 0)
                {
                    perror("recv");
                    pthread_exit(NULL);
                }
                filename[len] = '\0';
                // read the file
                FILE *fp = fopen(filename, "r");
                if (NULL == fp)
                {
                    perror("fopen");
                    pthread_exit(NULL);
                }
                // get the file size
                fseek(fp, 0, SEEK_END);
                long file_size = ftell(fp);
                fseek(fp, 0, SEEK_SET);
                // calculate the checksum
                char *checksum = checksum_file((algo_type_t)(atoi(algo)), (char *)fp, file_size, filename);
                len = send(client_sock, checksum, strlen(checksum), 0);
                if (len < 0)
                {
                    perror("send");
                    pthread_exit(NULL);
                }
                // free(checksum);
                fclose(fp);
                if (strcmp(checksum_client, checksum) == 0)
                {
                    len = send(client_sock, "OK", sizeof("OK"), 0);
                    if (len < 0)
                    {
                        perror("send");
                        pthread_exit(NULL);
                    }
                }
                else
                {
                    len = send(client_sock, "NOK", sizeof("NOK"), 0);
                    if (len < 0)
                    {
                        perror("send");
                        pthread_exit(NULL);
                    }
                }
            }
        }
    }

    pthread_exit(NULL);
}