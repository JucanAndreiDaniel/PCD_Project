#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <sys/un.h>

int main(int argc, char *argv[])
{
    // connect to server with unix socket

    int fd;

    struct sockaddr_un addr;
    socklen_t addr_len = sizeof(addr);

    struct sockaddr_un from;
    socklen_t from_len = sizeof(from);

    int ret;
    char buff[8192];
    int ok = 1;
    int len;

    char clientSocket[] = "/tmp/client.sock";
    char serverSocket[] = "/tmp/server.sock";

    if ((fd = socket(PF_UNIX, SOCK_DGRAM, 0)) < 0)
    {
        perror("socket");
        ok = 0;
    }

    if (ok)
    {
        memset(&addr, 0, sizeof(addr));
        addr.sun_family = AF_UNIX;
        strcpy(addr.sun_path, clientSocket);
        unlink(clientSocket);
        if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
        {
            perror("bind");
            ok = 0;
        }
    }

    if (ok)
    {
        memset(&addr, 0, sizeof(addr));
        addr.sun_family = AF_UNIX;
        strcpy(addr.sun_path, serverSocket);
        if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
        {
            perror("connect");
            printf("Could not connect to server.\n");
            ok = 0;
        }
    }
    if (ok)
    {
        strcpy(buff, "iccExchangeAPDU");
        if (send(fd, buff, strlen(buff) + 1, 0) == -1)
        {
            perror("send");
            printf("Could not send message.\n");
            ok = 0;
        }
        printf("sent iccExchangeAPDU\n");
    }

    if (ok)
    {
        struct timeval tv;
        tv.tv_sec = 5;
        tv.tv_usec = 0;
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(fd, &readfds);
        ret = select(fd + 1, &readfds, NULL, NULL, &tv);
        if (ret == 0)
        {
            printf("timeout\n");
            ok = 0;
        }
        else if (ret == -1)
        {
            perror("select");
            ok = 0;
        }
        else
        {
            if (FD_ISSET(fd, &readfds))
            {
                len = recv(fd, buff, sizeof(buff), 0);
                if (len == -1)
                {
                    perror("recv");
                    ok = 0;
                }
                else
                {
                    buff[len] = 0;
                    printf("received %s\n", buff);
                }
            }
        }

        printf("receive %d %s\n", len, buff);
    }

    while (ok)
    {
        printf("\n");
        printf("1. Get Logs\n");
        printf("2. Get Average running time\n");
        printf("3. Get Total number of requests\n");
        printf("0. Exit\n");

        printf("\n");
        printf("Enter your choice: ");
        int choice;
        scanf("%d", &choice);

        // send choice as string to server
        strcpy(buff, "");
        sprintf(buff, "%d", choice);
        if (send(fd, buff, strlen(buff) + 1, 0) == -1)
        {
            perror("send");
            printf("Could not send message.\n");
            ok = 0;
        }

        system("clear");

        printf("\n");
        switch (choice)
        {
        case 1:
            printf("Logs:\n");
            break;
        case 2:
            printf("Average running time:");
            break;
        case 3:
            printf("Total number of requests:");
            break;
        case 0:
            printf("Exiting...\n");
            send(fd, "exit", strlen("exit") + 1, 0);
            ok = 0;
            close(fd);
            return;
        }
        while (len = recv(fd, buff, 8192, 0))
        {
            if (strcmp(buff, "end") == 0)
            {
                break;
            }
            printf("%s", buff);
        }
    }

    if (fd >= 0)
    {
        close(fd);
    }

    unlink(clientSocket);
    return 0;
}