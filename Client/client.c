#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>

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
    // TODO check for algo type and send it to server
    // TODO server like error checking
    // check if the user has entered the correct number of arguments
    if (argc != 2)
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

    send_file(fp, sockfd);
    printf("[+]File data sent successfully.\n");

    printf("[+]Closing the connection.\n");
    close(sockfd);

    return 0;
}