#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include "../shared/types.h"

#define SIZE (1024)

void send_file(FILE *fp, int sockfd)
{
    int n;
    char data[SIZE] = {0};

    while (fgets(data, SIZE, fp) != NULL)
    {
        int data_length=strlen(data);
        // before sending the actual data lets send the lenght of it
        // in order to compute the checksum only on useful data
        if (send(sockfd, &data_length, sizeof(int), 0) == -1)
        {
            perror("[-]Error in sending data_length.");
            exit(1);
        }

        if (send(sockfd, data, strlen(data), 0) == -1)
        {
            perror("[-]Error in sending file.");
            exit(1);
        }
        printf("%d sizeof \n", strlen(data));
        bzero(data, SIZE);
    }
}

int main(int argc, char *argv[])
{
    // TODO server like error checking

    // check if the user has entered the correct number of arguments
    if (argc != 3)
    {
        printf("[-]Usage: %s <filename>\n", argv[0]);
        exit(1);
    }

    //check if the file exists
    char *filename = argv[1];
    FILE *fp = fopen(filename, "r");
    if (fp == NULL)
    {
        printf("[-]Error in opening file.\n");
        exit(1);
    }

    char *ip = "127.0.0.1";
    int port = 8080;
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

    // check for correct number value
    algo_type_t option = atoi(argv[2]);

    send(sockfd, &option, sizeof(algo_type_t), 0);
    printf("[+]Send algo type.\n");


    send_file(fp, sockfd);
    printf("[+]File data sent successfully.\n");
    char stop_send[] = "end";

    send(sockfd, &stop_send, sizeof(stop_send), 0);

    uint8_t digest[digest_size_list[(uint8_t)option]];
    memset(digest, 0, sizeof(digest));

    recv(sockfd, digest, sizeof(uint8_t) * digest_size_list[(uint8_t)option], 0);

    for (int i = 0; i < digest_size_list[(uint8_t)option]; i++)
    {
        printf("%02x", digest[i]);
    }
    printf("\n");

    printf("[+]Closing the connection.\n");
    close(sockfd);

    return 0;
}