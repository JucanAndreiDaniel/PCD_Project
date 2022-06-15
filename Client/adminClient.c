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
        if ((len = recv(fd, buff, 8192, 0)) < 0)
        {
            perror("recv");
            printf("Could not receive message.\n");
            ok = 0;
        }
        printf("receive %d %s\n", len, buff);
    }

    while (ok)
    {
        printf("\n");
        printf("1. Get Logs\n");
        printf("2. Get Average running time\n");
        printf("3. Get Total running time\n");
        printf("4. Get Total number of requests\n");
        printf("5. Get Threads Currently running\n");
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
            printf("Average running time:\n");
            break;
        case 3:
            printf("Total running time:\n");
            break;
        case 4:
            printf("Total number of requests:\n");
            break;
        case 5:
            printf("Threads Currently running:\n");
            break;
        case 0:
            printf("Exiting...\n");
            send(fd, "exit", strlen("exit") + 1, 0);
            ok = 0;
            close(fd);
        }
        printf("\n");
        while (len = recv(fd, buff, 8192, 0))
        {
            printf("%s", buff);
            if (strcmp(buff, "end") == 0)
            {
                break;
            }
        }
        // wait for enter key
        getchar();
    }

    if (fd >= 0)
    {
        close(fd);
    }

    unlink(clientSocket);
    return 0;
    // menu for user to choose which option to use
}