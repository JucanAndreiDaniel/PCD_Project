#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <openssl/sha.h>
#include <openssl/md5.h>
#include <openssl/aes.h>
#include <pthread.h>
#include <unistd.h>
#include "../shared/Utils.h"
#include "../shared/types.h"

// gcc -o server server.c -lcrypto -lssl

#define SIZE (1024) // this should be changed
#define GENERIC_CONTEXT_SIZE (2 * 1024) // 2048 bytes (2KB) should be enought for any context
#define TEST_ALGO (4)


///////// ------- [Global Data]
int thread_count = 0;
static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
static char file_name[] = "log.txt";



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

void call_sum_update(algo_type_t algo, void *CTX, uint8_t *data, long data_len)
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

void receive_data(int sockfd)
{

    int n;
    uint32_t total_size = 0;
    char buffer[SIZE];
    void *ctx = malloc(GENERIC_CONTEXT_SIZE);

    if(NULL == ctx)
    {
        return;
    }

    algo_type_t option;
    (void)recv(sockfd, &option, sizeof(algo_type_t), 0);
    printf("%d ###\n", option);

    call_sum_init(option, ctx);

    while (1)
    {
        int data_len = 0;
        (void)recv(sockfd, &data_len, sizeof(int), 0);

        total_size +=data_len;

        n = recv(sockfd, buffer, data_len, 0);
        if (n <= 0 || strcmp(buffer, "end"))
        {
            break;
        }

        call_sum_update(option, (void *)ctx, buffer, data_len);

        bzero(buffer, SIZE);
    }
    uint8_t digest[digest_size_list[(uint8_t)option]];

    call_sum_finale(option, (void *)ctx, digest);

    for (int i = 0; i < digest_size_list[(uint8_t)option]; i++)
    {
        DBG_PRINT("%02x", digest[i]);
    }
    DBG_PRINT("\n");

    pthread_mutex_lock(&mtx);
    FILE *fp;
    fp = fopen(file_name, "a"); // open file to append it

    if(NULL != fp)
    {
        fprintf(fp, "%d;", (int)option);

        fflush(fp);

        for (int i = 0; i < digest_size_list[(uint8_t)option]; i++)
        {
            fprintf(fp,"%02x", digest[i]);

            fflush(fp);
        }

        fprintf(fp, ";%u\n", total_size);

        fflush(fp);

        fclose(fp);
    }

    pthread_mutex_unlock(&mtx);


    send(sockfd, digest, sizeof(uint8_t) * digest_size_list[(uint8_t)option], 0);

    if(NULL != ctx)
    {
        free (ctx);
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

int main()
{
    char *ip = "127.0.0.1";
    int port = 8080;

    FILE *fp;
    fp = fopen(file_name, "w"); // just truncate/create for the current session
    fclose(fp);

    int sockfd, new_sock;
    struct sockaddr_in server_addr, new_addr;
    socklen_t addr_size = sizeof(new_addr);

    CHECK_SET_NO_ERR(socket(AF_INET, SOCK_STREAM, 0), NEGATIVE_ERR, sockfd, "[-]Error in socket\n");

    DBG_PRINT("[+]Server socket created successfully.\n");

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = port;
    server_addr.sin_addr.s_addr = inet_addr(ip);

    CHECK_ERR(bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)),
              OK,
              "[+]Binding successfull.\n",
              "[-]Error in bind\n");

    pthread_t pid[100];
    th_data new_th[100];

    CHECK_ERR(listen(sockfd, 32), OK, "[+]Listening....\n", "[-]Error in listening\n");
    while (1)
    {
        new_sock = accept(sockfd, (struct sockaddr *)&new_addr, &addr_size);
        CHECK_NO_ERR(new_sock, NEGATIVE_ERR, "[+]Accept\n", "[-]Error accepting\n");

        new_th[thread_count].thid = thread_count;
        new_th[thread_count].new_sock = new_sock;



        CHECK_ERR(pthread_create(&pid[thread_count], NULL, &threadFunction, &new_th[thread_count]),
                  OK,
                  "[+]Thread created successfully.\n",
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