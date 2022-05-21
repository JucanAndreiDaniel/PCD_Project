#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <openssl/sha.h>
#include <pthread.h>
// gcc -o server server.c -lcrypto -lssl

#define SIZE (1024)

#define DBG_PRINT_ENABLED
#if (defined(DBG_PRINT_ENABLED))
#define DBG_PRINT(...) printf(__VA_ARGS__)
#else
#define DBG_PRINT(...)
#endif

typedef enum
{
    SHA256_T = 1,
} algo_type_t;

void call_sum_init(algo_type_t algo, void *CTX)
{
    switch (algo)
    {
    case SHA256_T:
        SHA256_Init((SHA256_CTX *)CTX);
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
        (void)SHA256_Final((SHA256_CTX *)CTX, digest);
        break;
    default:
        DBG_PRINT("default for now\n");
        break;
    }
}

void receive_data(int sockfd)
{

    int n;
    FILE *fp;
    char *filename = "recv.txt";
    char buffer[SIZE];
    SHA_CTX sha;
    call_sum_init(1, &sha);

    fp = fopen(filename, "w");
    while (1)
    {
        int data_len;
        (void)recv(sockfd, &data_len, sizeof(int), 0);

        n = recv(sockfd, buffer, data_len, 0);
        if (n <= 0)
        {
            break;
            return;
        }
        call_sum_update(1, (void *)&sha, buffer, data_len);
        fprintf(fp, "%s", buffer);
        bzero(buffer, SIZE);
    }
    uint8_t digest[256];

    call_sum_finale(1, (void *)&sha, digest);

    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++)
    {
        DBG_PRINT("%02x", digest[i]);
    }
    DBG_PRINT("\n");
    return;
}

typedef struct
{
    int thid;
    int new_sock;
} th_data;

 th_data new_th[4];

void *thredFunction(void *arg)
{
    th_data *data_struct = (th_data *)arg;
    receive_data(data_struct->new_sock);

    return NULL;
}

int main()
{
    char *ip = "127.0.0.1";
    int port = 8080;
    int e;

    int sockfd, new_sock;
    struct sockaddr_in server_addr, new_addr;
    socklen_t addr_size;
    char buffer[SIZE];

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("[-]Error in socket");
        exit(1);
    }
    DBG_PRINT("[+]Server socket created successfully.\n");

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = port;
    server_addr.sin_addr.s_addr = inet_addr(ip);

    e = bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (e < 0)
    {
        perror("[-]Error in bind");
        exit(1);
    }
    DBG_PRINT("[+]Binding successfull.\n");

    int thread_count = 0;
    pthread_t pid[4];


    // while (1)
    // {
        if (listen(sockfd, 10) == 0)
        {
            DBG_PRINT("[+]Listening....\n");
        }
        else
        {
            perror("[-]Error in listening");
            exit(1);
        }
        addr_size = sizeof(new_addr);
        new_sock = accept(sockfd, (struct sockaddr *)&new_addr, &addr_size);

        new_th[thread_count].thid = thread_count;
        new_th[thread_count].new_sock = new_sock;

        pthread_create(&pid[thread_count], NULL, &thredFunction, &new_th[thread_count]);
        thread_count++;


        DBG_PRINT("[+]Data written in the file successfully.\n");
    // }

    for(int wait=0; wait < thread_count; wait++)
    {
        pthread_join(pid[wait], NULL);
    }



    return 0;
}