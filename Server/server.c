#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <openssl/sha.h>
// gcc -o server server.c -lcrypto -lssl
#define SIZE (1024)
void write_file(int sockfd)
{
    int n;
    FILE *fp;
    char *filename = "recv.txt";
    char buffer[SIZE];
    SHA_CTX sha;
    SHA256_Init(&sha);

    fp = fopen(filename, "w");
    while (1)
    {
        int data_len;
        (void)recv(sockfd, &data_len, sizeof(int),0);

        n = recv(sockfd, buffer, data_len, 0);
        if (n <= 0)
        {
            break;
            return;
        }
        SHA256_Update(&sha, buffer, data_len);
        printf("recv 1 byte\n");
        fprintf(fp, "%s", buffer);
        bzero(buffer, SIZE);
    }
    uint8_t digest[256];

    SHA256_Final(digest,&sha);

    for(int i=0;i<SHA256_DIGEST_LENGTH;i++)
    {
        printf("%02x",digest[i]);
    }
    printf("\n");
    return;
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
    printf("[+]Server socket created successfully.\n");

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = port;
    server_addr.sin_addr.s_addr = inet_addr(ip);

    e = bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (e < 0)
    {
        perror("[-]Error in bind");
        exit(1);
    }
    printf("[+]Binding successfull.\n");

    if (listen(sockfd, 10) == 0)
    {
        printf("[+]Listening....\n");
    }
    else
    {
        perror("[-]Error in listening");
        exit(1);
    }

    addr_size = sizeof(new_addr);
    new_sock = accept(sockfd, (struct sockaddr *)&new_addr, &addr_size);
    write_file(new_sock);
    printf("[+]Data written in the file successfully.\n");

    return 0;
}