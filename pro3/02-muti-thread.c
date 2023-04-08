//多线程版本的高并发服务器
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<ctype.h>
#include<pthread.h>
#include"wrap.h"


typedef struct info
{
    int cfd;//若为-1表示可用，大于0表示已被占用
    int idx;//当前子线程的下标
    pthread_t thread;//线程标识符
    struct sockaddr_in client; //客户端的socket地址
}INFO;

INFO thInfo[1024];

//子线程回调函数  线程执行函数
void *thread_work(void *arg)
{
    INFO *p=(INFO*) arg;//强制转换参数类型
    printf("idx==[%d]\n",p->idx);//打印当前线程的下标

    char sIP[16];
    memeset(sIP,0x00,sizeof(sIP));//清空缓存
    printf("new client:[%s][%d]\n",inet_ntop(AF_INET,
                                                &(p->client.sin_addr.s_addr),
                                                sIP,
                                                sizeof(sIP)),
                                    ntohs(p->client.sin_port));
    //将网络字节序IP地址转换成字符串格式，打印客户端的IP地址和端口号

    int cfd=p->cfd;//获取当前子线程的通信文件描述符
    int n;
    struct sockaddr_in client;
    memcpy(&client,&(p->client),sizeof(client));//拷贝客户端的socket地址到局部变量client

    char buf[1024];
    while(1){
        //read数据
        memset(buf,0x00,sizeof(buf));       
        n=Read(cfd,buf,sizeof(buf));//读取数据
        if(n<=0){//如果读取出错或客户端关闭连接
            printf("read error or client closed,n=[%d]\n",n);
            Close(cfd);
            p->cfd=-1;
            pthread_exit(NULL);
        }

        printf("n==[%d],buf==[%s]",n,buf);

        for(int i=0;i<n;i++){
            buf[i]=toupper(buf[i]);
        }
        //发送数据
        Write(cfd,buf,n);

    }
    //关闭通讯描述符
    close(cfd);
    pthread_exit(NULL);
}

void init_thInfo(){
    int i=0;
    for(i=0;i<1024;i++){
        thInfo[i].cfd=-1;
    }
}


int findIndex()
{
    int i;
    for(i =0;i<1024;i++){
        if(thInfo[i].cfd==-1)
        {
            break;
        }
    }
    if(i==1024){
        return -1;
    }
    return i;


}

int main()
{
    //创建socket
    int lfd=Socket(AF_INET,SOCK_STREAM,0);

    //设置端口复用
    int opt=1;
    setsockopt(lfd,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(int));


    //绑定
    struct sockaddr_in serv;
    bzero(&serv,sizeof(serv));
    serv.sin_family=AF_INET;
    serv.sin_port=htons(9999);
    serv.sin_addr.s_addr=htonl(INADDR_ANY);
    Bind(lfd,(struct sockaddr *)&serv,sizeof(serv));

    //设置监听
    Listen(lfd,128);

    int cfd;
    struct sockaddr_in client;
    socklen_t len;
    pthread_t thread;
    int idx;
    int ret;
    while(1)
    {
        len=sizeof(client);
        bzero(&client,sizeof(client));
        //接受新的连接
        cfd=Accept(lfd,(struct sockaddr *)&client,len);

        //找数组中空闲的位置
        idx=findIndex();
        if(idx==-1)
        {
            Close(cfd);
            continue;
        }

        //对空闲位置的元素成员赋值
        thInfo[idx].cfd=cfd;
        thInfo[idx].idx=idx;
        memcpy(&thInfo[idx].client,&client,sizeof(client));
        

        //创建子线程--该子线程完成对数据的收发
        pthread_create(&thInfo[idx].thread,NULL,thread_work,&cfd);
        if(ret!=0){
            printf("create thread error:[%s]\n",strerror(ret));
            exit(-1);
        }
        //设置子线程为分离属性
        pthread_detach(thread);


    }

   

    //关闭监听文件描述符
    close(lfd);

    return 0;
}