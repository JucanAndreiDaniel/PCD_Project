#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

#define SIZE (1024)

int unix_socket(const char *filename)
{
    int fd;
    struct sockaddr_un addr;
    int ok = 1;
    if ((fd = socket(PF_UNIX, SOCK_DGRAM, 0)) < 0)
    {
        perror("socket");
        ok = 0;
    }

    if (ok)
    {
        memset(&addr, 0, sizeof(addr));
        addr.sun_family = AF_UNIX;
        strcpy(addr.sun_path, filename);
        unlink(filename);
        if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
        {
            perror("bind");
            ok = 0;
        }
        printf("Bound to %s\n", filename);
    }
    return fd;
}

void *unix_main(void *args)
{
    char *socket = (char *)args;
    int sock = unix_socket(socket);
    struct sockaddr_un client;
    socklen_t client_len = sizeof(client);
    char buffer[SIZE];
    int ret;
    int len;

    printf("Waiting for client...\n");
    len = recvfrom(sock, buffer, SIZE, 0, (struct sockaddr *)&client, &client_len);
    printf("Connected to Admin Client: %s\n", buffer);
    strcpy(buffer, "transmit good!");
    ret = sendto(sock, buffer, strlen(buffer) + 1, 0, (struct sockaddr *)&client, client_len);
    if (ret < 0)
    {
        perror("sendto");
        return;
    }
    bzero(buffer, SIZE);

    while ((len = recvfrom(sock, buffer, SIZE, 0, (struct sockaddr *)&client, &client_len)) > 0)
    {
        if (strcmp(buffer, "0") == 0)
        {
            pthread_exit(NULL);
        }
        else if (strcmp(buffer, "1") == 0)
        {
            FILE *fp = fopen("log.txt", "r");
            if (fp == NULL)
            {
                perror("fopen");
                pthread_exit(NULL);
            }
            char line[SIZE];
            while (1)
            {
                fgets(line, SIZE, fp);
                sendto(sock, line, strlen(line) + 1, 0, (struct sockaddr *)&client, client_len);
                if (feof(fp) || line == NULL)
                {
                    break;
                }
            }
            sendto(sock, "end", strlen("end") + 1, 0, (struct sockaddr *)&client, client_len);
            fclose(fp);
        }
        else if (strcmp(buffer, "2") == 0)
        {
            FILE *fp = fopen("log.txt", "r");
            if (fp == NULL)
            {
                perror("fopen");
                pthread_exit(NULL);
            }
            char line[SIZE];
            float total = 0;
            int count = 0;
            while (1)
            {
                fgets(line, SIZE, fp);
                if (feof(fp) || line == NULL)
                {
                    break;
                }
                if (line != NULL)
                {
                    char *time = strtok(line, ";");
                    char *last_time = time;
                    while (time != NULL)
                    {
                        last_time = time;
                        time = strtok(NULL, ";");
                    }
                    total = total + atof(last_time);
                }
                count++;
            }
            if (total == 0)
                total = 1;
            float avg = (float)(total / count);

            // avg to string
            char avg_str[10];
            sprintf(avg_str, "%f", avg);

            fclose(fp);
            sendto(sock, &avg_str, sizeof(avg_str), 0, (struct sockaddr *)&client, client_len);
            sendto(sock, "end", strlen("end") + 1, 0, (struct sockaddr *)&client, client_len);
        }
        else if (strcmp(buffer, "3") == 0)
        {
            FILE *fp = fopen("log.txt", "r");
            if (fp == NULL)
            {
                perror("fopen");
                pthread_exit(NULL);
            }
            char line[SIZE];
            int count = 0;
            while (1)
            {
                fgets(line, SIZE, fp);
                if (feof(fp) || line == NULL)
                {
                    break;
                }
                if (line != NULL)
                {
                    count++;
                }
            }
            fclose(fp);

            // count to string
            char count_str[10];
            sprintf(count_str, "%d", count);

            sendto(sock, &count_str, sizeof(count_str), 0, (struct sockaddr *)&client, client_len);
            sendto(sock, "end", strlen("end"), 0, (struct sockaddr *)&client, client_len);
        }
    }

    if (sock >= 0)
    {
        close(sock);
    }
}