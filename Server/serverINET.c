#include <arpa/inet.h>

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "include/checksum.h"

// gcc -o server server.c -lcrypto -lssl

#define SIZE (1024) // this should be changed
#define GENERIC_CONTEXT_SIZE \
    (2 * SIZE) // 2048 bytes (2KB) should be enought for any context
#define TEST_ALGO (4)

///////// ------- [Global Data]
int thread_count = 0;
static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
static char file_name[] = "log.txt";

void receive_data(int sockfd)
{
    uint32_t digest_size_list[ALGO_NUM] = {(uint32_t)MD5_SZ, (uint32_t)SHA_SZ, (uint32_t)SHA224_SZ, (uint32_t)SHA256_SZ, (uint32_t)SHA384_SZ, (uint32_t)SHA512_SZ};

    int n;
    uint32_t total_size = 0;

    sum_operation_t operation;
    (void)recv(sockfd, &operation, sizeof(sum_operation_t), 0);

    algo_type_t option;
    (void)recv(sockfd, &option, sizeof(algo_type_t), 0);

    uint32_t data_size;
    (void)recv(sockfd, &data_size, sizeof(uint32_t), 0);

    uint8_t buffer[SIZE];

    void *ctx = malloc(GENERIC_CONTEXT_SIZE);

    if (NULL == ctx)
    {
        return;
    }

    call_sum_init(option, ctx);

    while (1)
    {
        uint32_t data_len = 0;
        (void)recv(sockfd, &data_len, sizeof(uint32_t), 0);
        if (data_len > SIZE)
        {
            break;
        }
        total_size += data_len;

        n = recv(sockfd, buffer, data_len, 0);
        if (n <= 0 || strcmp(buffer, "end") == 0)
        {
            printf("[-]Error in receiving data.\n");
            break;
        }
        call_sum_update(option, (void *)ctx, buffer, data_len);

        bzero(buffer, SIZE);
    }
    uint8_t digest[digest_size_list[(uint8_t)option]];

    call_sum_finale(option, (void *)ctx, digest);

    char *checksum_str = malloc(sizeof(char) * digest_size_list[(uint8_t)option] * 2 + 1);
    for (int i = 0, j = 0; i < digest_size_list[(uint8_t)option]; i++, j += 2)
        sprintf(checksum_str + j, "%02x", digest[i]);
    checksum_str[sizeof(char) * digest_size_list[(uint8_t)option] * 2] = 0;

    clock_t start = clock();

    clock_t end = clock();

    float seconds = (float)(end - start) / CLOCKS_PER_SEC;

    pthread_mutex_lock(&mtx);

    writeLog(file_name, option, checksum_str, total_size, seconds);

    pthread_mutex_unlock(&mtx);

    if (CHECKSUM_OP == operation)
    {
        send(sockfd, checksum_str, sizeof(char *) * digest_size_list[(uint8_t)option] * 2 + 1, 0);
    }
    else
    {
        uint8_t cipher[digest_size_list[(uint8_t)option]];

        recv(sockfd, cipher, sizeof(char) * digest_size_list[(uint8_t)option] * 2 + 1, 0);

        int response = strcmp(checksum_str, cipher);

        send(sockfd, &response, sizeof(int), 0);
    }

    if (NULL != ctx)
    {
        free(ctx);
        ctx = NULL;
    }
    if (NULL != checksum_str)
    {
        free(checksum_str);
        checksum_str = NULL;
    }

    return;
}

typedef struct
{
    int thid;
    int new_sock;
} th_data;

void *threadFunction(void *arg)
{
    th_data *data_struct = (th_data *)arg;
    receive_data(data_struct->new_sock);

    close(data_struct->new_sock);

    pthread_t id = pthread_self();

    pthread_mutex_lock(&mtx);
    thread_count--;
    pthread_mutex_unlock(&mtx);

    pthread_exit(id);

    return NULL;
}

void *inet_main(void *args)
{
    char *ip = "127.0.0.1";
    int port = *((int *)args);

    int sockfd, new_sock;
    struct sockaddr_in server_addr, new_addr;
    socklen_t addr_size = sizeof(new_addr);

    CHECK_SET_NO_ERR(socket(AF_INET, SOCK_STREAM, 0), NEGATIVE_ERR, sockfd,
                     "[-]Error in socket\n");

    DBG_PRINT("[+]Server socket created successfully.\n");

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = port;
    server_addr.sin_addr.s_addr = inet_addr(ip);

    CHECK_ERR(bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)),
              OK, "[+]Binding successfull.\n", "[-]Error in bind\n");

    pthread_t pid[100];
    th_data new_th[100];

    CHECK_ERR(listen(sockfd, 32), OK, "[+]Listening....\n",
              "[-]Error in listening\n");
    while (1)
    {
        new_sock = accept(sockfd, (struct sockaddr *)&new_addr, &addr_size);
        CHECK_NO_ERR(new_sock, NEGATIVE_ERR, "[+]Accept\n", "[-]Error accepting\n");

        new_th[thread_count].thid = thread_count;
        new_th[thread_count].new_sock = new_sock;

        CHECK_ERR(pthread_create(&pid[thread_count], NULL, &threadFunction,
                                 &new_th[thread_count]),
                  OK, "[+]Thread created successfully.\n",
                  "[-]Thred create error\n");

        pthread_mutex_lock(&mtx);
        thread_count++;
        pthread_mutex_unlock(&mtx);

        printf("thread_count %d\n", thread_count);
    }

    for (int wait = 0; wait < thread_count; wait++)
    {
        pthread_join(pid[wait], NULL);
    }

    return 0;
}