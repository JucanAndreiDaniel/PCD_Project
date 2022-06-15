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
    char buffer[8196];
    int n;
    int ret;
    int len;

    printf("Waiting for client...\n");
    len = recvfrom(sock, buffer, 8192, 0, (struct sockaddr *)&client, &client_len);
    printf("Connected to Admin Client: %s\n", buffer);
    strcpy(buffer, "transmit good!");
    ret = sendto(sock, buffer, strlen(buffer) + 1, 0, (struct sockaddr *)&client, client_len);
    if (ret < 0)
    {
        perror("sendto");
        return;
    }
    bzero(buffer, 8192);

    while ((len = recvfrom(sock, buffer, 8196, 0, (struct sockaddr *)&client, &client_len)) > 0)
    {
        // if buffer is exit or quit then exit
        printf("%s\n", buffer);
        if (strcmp(buffer, "0") == 0)
        {
            pthread_exit(NULL);
        }
        // check if buffer is a command
        if (strcmp(buffer, "help") == 0)
        {
            printf("\n");
            printf("1. Get Logs\n");
            printf("2. Get Average running time\n");
            printf("3. Get Total running time\n");
            printf("4. Get Total number of requests\n");
            printf("5. Get Threads Currently running\n");
            printf("0. Exit\n");
        }
        else if (strcmp(buffer, "1") == 0)
        {
            FILE *fp = fopen("log.txt", "r");
            if (fp == NULL)
            {
                perror("fopen");
                pthread_exit(NULL);
            }
            char line[1024];
            while (1)
            {
                fgets(line, 1024, fp);
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
            char line[2048];
            float total = 0;
            int count = 0;
            while (1)
            {
                fgets(line, 2048, fp);
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
            fclose(fp);
            printf("Average running time: %f\n", avg);
            sendto(sock, "Average running time: ", strlen("Average running time: ") + 1, 0, (struct sockaddr *)&client, client_len);
            sendto(sock, &avg, sizeof(avg) + 1, 0, (struct sockaddr *)&client, client_len);
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
            char line[1024];
            float total = 0;
            while (1)
            {
                fgets(line, 2048, fp);
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
            }
            fclose(fp);
            printf("Total running time: %f\n", total);
            sendto(sock, "Total running time: ", strlen("Total running time: "), 0, (struct sockaddr *)&client, client_len);
            sendto(sock, &total, sizeof(total), 0, (struct sockaddr *)&client, client_len);
            sendto(sock, "end", strlen("end"), 0, (struct sockaddr *)&client, client_len);
        }
        else if (strcmp(buffer, "4") == 0)
        {
            FILE *fp = fopen("log.txt", "r");
            if (fp == NULL)
            {
                perror("fopen");
                pthread_exit(NULL);
            }
            char line[1024];
            int count = 0;
            while (1)
            {
                fgets(line, 2048, fp);
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
            printf("Total number of requests: %d\n", count);
            sendto(sock, "Total number of requests: ", strlen("Total number of requests: "), 0, (struct sockaddr *)&client, client_len);
            sendto(sock, &count, sizeof(count), 0, (struct sockaddr *)&client, client_len);
            sendto(sock, "end", strlen("end"), 0, (struct sockaddr *)&client, client_len);
        }
        else if (strcmp(buffer, "getCurrentlyRunning") == 0)
        {
            // print threads that are running
            // pthread_mutex_lock(&curmtx);
            // printf("Currently running threads:\n");
            // for (int i = 0; i < curthreads; i++)
            // {
            //     printf("%d\n", curthreads[i]);
            // }
            // pthread_mutex_unlock(&curmtx);
        }
        // average running time
    }

    if (sock >= 0)
    {
        close(sock);
    }
}