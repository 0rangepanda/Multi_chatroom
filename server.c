#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <errno.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <time.h>

#define LISTENQ 5
#define MAXLINE 512
#define MAXMEM 10
#define NAMELEN 20

int listenfd,connfd[MAXMEM];//�ֱ��¼�������˵��׽��������ӵĶ���ͻ��˵��׽���

void quit();//�������رպ���
void rcv_snd(int n);//���������ղ�ת����Ϣ����

char **namelist;
int portlist[MAXMEM];

int main()
{
    pthread_t thread;
    struct sockaddr_in servaddr,cliaddr;
    socklen_t len;
    time_t ticks;
    char buff[MAXLINE];

//�û����б�
    //char **namelist = malloc(sizeof *namelist * MAXMEM);
    namelist = malloc(sizeof *namelist * MAXMEM);
    if (namelist)
    {
      size_t in;
      for (in = 0; in < MAXMEM; in++)
      {
        namelist[in] = malloc(sizeof *namelist[in] * (NAMELEN + 1));
        //strcpy(arr[i], "          ");
      }
    }

//����socket���������������˵��׽���
    printf("Socket...\n");
    listenfd=socket(AF_INET,SOCK_STREAM,0);
    if(listenfd<0)
    {
        printf("Socket created failed.\n");
        return -1;
    }

//����bind����ʹ�÷������˵��׽������ַʵ�ְ�
    printf("Bind...\n");
    servaddr.sin_family=AF_INET;
    servaddr.sin_port=htons(6666);
    servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
    if(bind(listenfd,(struct sockaddr*)&servaddr,sizeof(servaddr))<0)
    {
        printf("Bind failed.\n");
        return -1;
    }

//����listen��������һ�����������׽��ֱ�Ϊ�����ļ����׽���
//�ڴ˹��������tcp��������������
    printf("listening...\n");
    listen(listenfd,LISTENQ);

//����һ���̣߳��Է�����������й����رգ�
    pthread_create(&thread,NULL,(void*)(&quit),NULL);

//��¼���еĿͻ��˵��׽�����������-1Ϊ���У�
    int i=0;
    for(i=0;i<MAXMEM;i++)
    {
        connfd[i]=-1;
    }

    while(1)
    {
        len = sizeof(cliaddr);
        for(i=0;i < MAXMEM; i++)
        {
            if(connfd[i]==-1)
            {
                break;
            }
        }
        //����accept��listen���ܵ����Ӷ�����ȡ��һ������
        connfd[i] = accept(listenfd,(struct sockaddr *) &cliaddr, &len);
        ticks=time(NULL);
        sprintf(buff,"%s \r \n",ctime(&ticks));
        //printf("%d\n",ntohs(cliaddr.sin_port));
        //printf("%s\n",inet_ntoa(cliaddr.sin_addr));
        printf("%s Connect from: %s,port %d\n\n",buff,inet_ntoa(cliaddr.sin_addr),ntohs(cliaddr.sin_port));
        portlist[i] = ntohs(cliaddr.sin_port);
        //��Ե�ǰ�׽��ִ���һ���̣߳��Ե�ǰ�׽��ֵ���Ϣ���д���
        pthread_create(malloc(sizeof(pthread_t)),NULL,(void*)(&rcv_snd),(void*)i);

    }

    return 0;
}


void quit()
{
    char msg[30];
    while(1)
    {
        scanf("%s",msg);
        if(strcmp("Cmd",msg)==0)
        {
            printf("Command List:\n");
            printf("[quit]:To stop server\n");
            printf("[name]:To show clients' name and port\n");
        }
        else if(strcmp("quit",msg)==0)
        {
            printf("Byebye...\n");
            close(listenfd);
            exit(0);
        }
        else if(strcmp("name",msg)==0)
        {
            printf("Name List:\n");
            size_t in;
            for (in = 0; in < MAXMEM; in++)
            {
                if (connfd[in]!=-1) {
                    printf("%s\t%d\n",namelist[in],portlist[in]);
                }
            }
        }
    }
}

void rcv_snd(int n)
{
    char* ask="Your name please��";
    char* used="Name already used!\n";
    char buff[MAXLINE];
    char buff1[MAXLINE];
    char buff2[MAXLINE];
    char name[NAMELEN];
    time_t ticks;
    int i=0;
    int retval;
    int len;

    //��ȡ�˽��̶�Ӧ���׽����û�������
    while (1) {
        write(connfd[n],ask,strlen(ask));
        len=read(connfd[n],name,NAMELEN);
        if(len>0)
        {
            name[len]=0;
            for (size_t in = 0; in < MAXMEM; in++)
            {
                //printf("%s\n",namelist[in]);
                if (strcmp(namelist[in], name)==0) {
                    write(connfd[n],used,strlen(used));
                    goto next;
                }
            }
            strcpy(namelist[n], name);
            break;
        }
        next:
        ;
    }

    //�ѵ�ǰ�û��ļ����֪�����û�
    strcpy(buff,name);
    strcat(buff,"\tjoin in\0");

    printf("%s\n",buff);

    for(i = 0 ;i < MAXMEM; i++)
    {
        if(connfd[i]!=-1)
        {
            write(connfd[i],buff,strlen(buff));
        }
    }

    //���ܵ�ǰ�û�����Ϣ������ת�������е��û�
    while(1)
    {
        if((len=read(connfd[n],buff1,MAXLINE))>0)
        {
            buff1[len]=0;

            //��ǰ�û���������ϢΪ��EXIT!��ʱ����ǰ�û��˳�
             if(strcmp("EXIT!",buff1)==0)
             {
                 printf("debug1\n");
                 strcpy(buff,name);
                 strcat(buff,"\thas exit!\0");
                 printf("%s\n",buff);

                 strcpy(namelist[n], "");

                 close(connfd[n]);
                 connfd[n]=-1;
                 pthread_exit(&retval);//�ر��߳�
             }

             ticks=time(NULL);
             sprintf(buff2,"%.24s\r\n",ctime(&ticks));
             write(connfd,buff2,strlen(buff2));

             strcpy(buff,name);
             strcat(buff,"\t");
             strcat(buff,buff2);
             strcat(buff,buff1);

             printf("%s\n",buff);

             for(i=0;i<MAXMEM;i++)
             {
                 if(connfd[i]!=-1)
                 {
                      write(connfd[i],buff,strlen(buff));
                 }
             }
        }

    }
}
