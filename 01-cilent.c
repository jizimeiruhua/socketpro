#include <stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<unistd.h>
#include<netinet/in.h>
#include<arpa/inet.h>
//客户端代码
int main(){
    //创建socket--用于和服务端进行通信
    int cfd=socket(AF_INET,SOCK_STREAM,0);
    if(cfd<0){
        perror("socket error");
        return -1;

    }
    //连接服务器
    //int connect(int sockfd,const struct sockaddr * addr,socklen_t addrlen);
    struct sockaddr_in serv;
    serv.sin_family=AF_INET;
    serv.sin_port=htons(9899);
    inet_pton(AF_INET,"127.0.0.1",&serv.sin_addr.s_addr);
    printf("[%x]\n",serv.sin_addr.s_addr);
    int ret=connect(cfd , (struct sockaddr* )&serv , sizeof(serv));
    if(ret<0){
        perror("connect error");
        return -1;
    }else{
        printf("connect complete\n");
    }

    int n=0;
    char buf[256];
    while(1){

        //读标准输入数据
        memset(buf,0x00,sizeof(buf));
        n=read(STDIN_FILENO,buf,sizeof(buf));
        //发送数据
        write(cfd,buf,n);

        //读服务端发来的数据
        memset(buf,0x00,sizeof(buf));
        n=read(cfd,buf,sizeof(buf));
        if(n<=0)
        {
            printf("read error or server closed,n==[%d]\n",n);
        }
        printf("n==[%d],buf=[%s]\n",n,buf);
    }
    //关闭套接字cfd
    close(cfd);
    return 0;
}