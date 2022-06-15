#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/sendfile.h>
#include "../shared/types.h"

#define SIZE (1024)

void send_file(FILE *fp, int sockfd, uint32_t file_size)
{
    int n;
    char data[SIZE];

    while (1)
    {
        // fread(data, SIZE, 1, fp);
        fgets(data, SIZE, fp);
        int data_length = strlen(data);

        if (data_length == 0 || data == NULL)
        {
            // printf("Error: data_length is above 1024\n");
            printf("Error: data_length is %d\n", data_length);
            printf("Error: data is %s\n", data);
            break;
        }
        printf("data_length is %d\n", data_length);
        if (send(sockfd, &data_length, sizeof(uint32_t), 0) == -1)
        {
            printf("Error: data_length is %d\n", data_length);
            printf("Error: data is %s\n", data);
            perror("[-]Error in sending data_length.");
            // exit(1);
        }

        printf("%d sizeof \n", strlen(data));
        printf("data is %s\n", data);
        if (send(sockfd, data, strlen(data), 0) == -1)
        {
            perror("[-]Error in sending file.");
            printf("Error: data_length is %d\n", data_length);
            printf("Error: data is %s\n", data);
            // exit(1);
        }
        bzero(data, SIZE);
    }
}

int main(int argc, char *argv[])
{
    uint32_t digest_size_list[ALGO_NUM] = {(uint32_t)MD5_SZ, (uint32_t)SHA_SZ, (uint32_t)SHA224_SZ, (uint32_t)SHA256_SZ, (uint32_t)SHA384_SZ, (uint32_t)SHA512_SZ};

    // check if the user has entered the correct number of arguments
    if (argc != 3)
    {
        printf("[-]Usage: %s <filename> <algo>\n", argv[0]);
        exit(1);
    }

    // check if the file exists
    char *filename = argv[1];
    FILE *fp = fopen(filename, "r");
    if (fp == NULL)
    {
        printf("[-]Error in opening file.\n");
        exit(1);
    }

    char *ip = "127.0.0.1";
    int port = 8081;
    int e;

    int sockfd;
    struct sockaddr_in server_addr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("[-]Error in socket");
        exit(1);
    }
    printf("[+]Server socket created successfully.\n");

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = port;
    server_addr.sin_addr.s_addr = inet_addr(ip);

    e = connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (e == -1)
    {
        perror("[-]Error in socket");
        exit(1);
    }
    printf("[+]Connected to Server.\n");

    // send operation
    sum_operation_t operation = (sum_operation_t)0;
    send(sockfd, &operation, sizeof(sum_operation_t), 0);

    // check for correct number value
    algo_type_t option = atoi(argv[2]);

    send(sockfd, &option, sizeof(algo_type_t), 0);
    printf("[+]Send algo type %d.\n", option);

    // send file size
    uint32_t file_size = 0;
    fseek(fp, 0, SEEK_END);
    file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    send(sockfd, &file_size, sizeof(uint32_t), 0);
    printf("[+]Sent file size %d.\n", file_size);

    send_file(fp, sockfd, file_size);

    char stop_send[] = "end";

    send(sockfd, stop_send, sizeof(stop_send), 0);

    uint8_t digest[digest_size_list[(uint8_t)option]];
    memset(digest, 0, sizeof(digest));

    char *checksum = malloc(sizeof(char *) * digest_size_list[(uint8_t)option] * 2 + 1);
    recv(sockfd, checksum, sizeof(char *) * digest_size_list[(uint8_t)option] * 2, 0);

    printf("[+]Received checksum.\n");
    printf("%s\n", checksum);
    free(checksum);
    printf("[+]Closing the connection.\n");
    close(sockfd);

    return 0;
}