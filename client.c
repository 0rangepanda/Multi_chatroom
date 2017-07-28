#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <errno.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>

#define MAXLINE 512
#define NAMELEN 20
#define PORT 6666
#define HOST_ADDR "127.0.0.1"
//#define HOST_ADDR "192.168.0.44"

int sockfd;
void snd();

int main(int argc, char **argv)
{
    pthread_t thread;
    pthread_t thread_heart;//心跳包线程，用于防止非法退出
    struct sockaddr_in servaddr;
    char* host_ip;

//调用socket函数创建客户端的套接字
    sockfd=socket(AF_INET,SOCK_STREAM,0);
    if(sockfd<0)
    {
        printf("Socket create failed\n");
        return -1;
    }

//初始化服务器端的地址
    servaddr.sin_family=AF_INET;
    servaddr.sin_port=htons(PORT);

    if (argc==2){
        host_ip = argv[1];
        //argv[1] is ip of server
    }
    else
        host_ip = HOST_ADDR;

    if(inet_aton(host_ip,&servaddr.sin_addr)<0)
    {
        printf("inet_aton error.\n");
        return -1;
    }
    //printf("Welcome Mr. %s!\n",argv[2]);

//调用connect函数实现与服务器端建立连接
    printf("Connecting...\n");
    if(connect(sockfd,(struct sockaddr*)&servaddr,sizeof(servaddr))<0)
    {
        printf("Connect server failed.\n");
        return  -1;
    }

//从此处开始程序分做两个线程

//创建发送消息的线程，调用了发送消息的函数snd
    pthread_create(&thread,NULL,(void*)(&snd),NULL);

//从此处开始向下为接收消息的线程
    char buff[MAXLINE];
    int len;

    while(1)
    {
        if((len=read(sockfd,buff,MAXLINE))>0)
        {
            buff[len]=0;
            printf("\n%s\n\n",buff);
        }
    }

    return 0;
}

//发送消息的函数snd
void snd()
{
    char name[NAMELEN];
    char buff[MAXLINE];
    gets(name);
    write(sockfd,name,strlen(name));
    while(1)
    {
        gets(buff);
        write(sockfd,buff,strlen(buff));
        if(strcmp("EXIT!",buff)==0)
        {
            //sprintf(buff,"%s EXIT!\r\n",name);
            //write(sockfd,buff,strlen(buff));
            //close(sockfd);
            exit(0);
        }
    }
}
