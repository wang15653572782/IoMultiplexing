#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <errno.h>
int main()
{
    int lfd = socket(AF_INET, SOCK_STREAM, 0);
    if (lfd == -1)
    {
        perror("socket error");
        exit(1);
    }
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof serv_addr);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(9999);
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    int ret;
    if ((ret = bind(lfd, (struct sockaddr *)&serv_addr, sizeof serv_addr)) == -1)
    {
        perror("bind error");
        exit(1);
    }
    ret = listen(lfd, 128);
    if (ret == -1)
    {
        perror("listen error");
        exit(1);
    }
    int epfd = epoll_create(100);
    if (epfd == -1)
    {
        perror("epoll_creat error");
        exit(0);
    }
    struct epoll_event ev;
    ev.events = EPOLLIN | EPOLLET;
    ev.data.fd = lfd;
    ret = epoll_ctl(epfd, EPOLL_CTL_ADD, lfd, &ev);
    if (ret == -1)
    {
        perror("epoll_ctl_add error");
        exit(0);
    }
    struct epoll_event evs[1024];
    int size = sizeof(evs) / sizeof(struct epoll_event);
    while (1)
    {
        int readyNum = epoll_wait(epfd, evs, size, -1);
        for (int i = 0; i < readyNum; ++i)
        {
            int curfd = evs[i].data.fd;
            if (curfd == lfd)
            {
                int cfd = accept(lfd, NULL, NULL);
                int flag = fcntl(cfd, F_GETFL);
                flag |= O_NONBLOCK;
                fcntl(cfd, F_SETFL, flag);
                struct epoll_event ev;
                ev.events = EPOLLIN | EPOLLET;
                ev.data.fd = cfd;
                ret = epoll_ctl(epfd, EPOLL_CTL_ADD, cfd, &ev);
                if (ret == -1)
                {
                    perror("epoll_ctl_add accept cfd error");
                    exit(0);
                }
            }
            else
            {
                char buf[5];
                memset(buf, 0, sizeof buf);
                while (1)
                {
                    int len = recv(curfd, buf, sizeof buf, 0);
                    // 水平模式，只要读缓冲区还有数据，以后内核就会一直通知到evs中
                    // 边沿模式，不管我们用户一次有没有读完，只有下次数据到达后才会接着通知到evs,不然通知一次就不会继续通知了
                    if (len == 0)
                    {
                        printf("client had disconnect\n");
                        epoll_ctl(epfd, EPOLL_CTL_DEL, curfd, NULL);
                        close(curfd);
                    }
                    else if (len > 0)
                    {
                        printf("client say: %s\n", buf);
                        write(STDOUT_FILENO, buf, len);
                        send(curfd, buf, len, 0);
                    }
                    else
                    {
                        if (errno == EAGAIN)
                        {
                            printf("数据读完了...\n");
                            break;
                        }
                        else
                        {
                            perror("recv error");
                            exit(0);
                        }
                    }
                }
            }
        }
    }
    return 0;
}
