#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>

int main()
{
    // 1. 创建通信的套接字
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(fd == -1)
    {
        perror("socket");
        exit(0);
    }

    // 2. 设置广播属性
    int opt  = 1;
    setsockopt(fd, SOL_SOCKET, SO_BROADCAST, &opt, sizeof(opt));

    char buf[1024];
    struct sockaddr_in cliaddr;
    int len = sizeof(cliaddr);
    cliaddr.sin_family = AF_INET;
    cliaddr.sin_port = htons(9999); // 接收端需要绑定9999端口
    // 只要主机在237网段, 并且绑定了9999端口, 这个接收端就能收到广播消息
    inet_pton(AF_INET, "172.19.159.255", &cliaddr.sin_addr.s_addr);
    // 3. 通信
    int num = 0;
    while(1)
    {
        sprintf(buf, "hello, client...%d\n", num++);
        // 数据广播
        sendto(fd, buf, strlen(buf)+1, 0, (struct sockaddr*)&cliaddr, len);
        printf("发送的广播的数据: %s\n", buf);
        sleep(1);
    }

    close(fd);

    return 0;
}

