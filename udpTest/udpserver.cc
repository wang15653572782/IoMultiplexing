#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>

int main()
{
    int fd=socket(AF_INET,SOCK_DGRAM,0);
    if(fd==-1)
    {
        perror("socket create fail!");
        exit(0);
    }
    struct sockaddr_in addr;
    addr.sin_family=AF_INET;
    addr.sin_port=htons(9999);
    addr.sin_addr.s_addr=INADDR_ANY;
    int ret=bind(fd,(struct sockaddr*)&addr,sizeof addr);
    if(ret==-1)
    {
        perror("fd bind fail!");
        exit(0);
    }
    char buf[1024];
    char ipbuf[64];
    struct sockaddr_in cliaddr;
    int len=sizeof(cliaddr);
    while(1)
    {
        memset(buf,0,sizeof buf);
        int rlen=recvfrom(fd,buf,sizeof buf,0,(struct sockaddr*)&cliaddr,(socklen_t*)&len);
        printf("客户端的IP地址:%s,端口:%d\n",
        inet_ntop(AF_INET, &cliaddr.sin_addr.s_addr, ipbuf, sizeof(ipbuf)),
        ntohs(cliaddr.sin_port));
        printf("客户端say:%s\n",buf);
        sendto(fd,buf,rlen,0,(struct sockaddr*)&cliaddr,sizeof cliaddr);
    }
    close(fd);
    return 0;
}