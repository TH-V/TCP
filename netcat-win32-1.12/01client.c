#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<fcntl.h>
#include<string.h>
#include<pthread.h>
#include<sys/socket.h>
#include<arpa/inet.h>

void sys_error(const char *str) 
{
    perror(str); // 打印错误信息
    exit(1); // 异常退出程序
}

int main(int argc, char *argv[])
{
    char buf[BUFSIZ]; // 定义缓冲区

    // 创建套接字
    int cfd = socket(AF_INET,SOCK_STREAM, 0);
    
    // 设置服务器地址
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(7777);
    inet_pton(AF_INET, "8.135.81.187", &server_addr.sin_addr.s_addr);
    
    // 连接服务器
    int ret = connect(cfd, (struct sockaddr*)&server_addr,sizeof server_addr);
    if(ret == -1 ) sys_error("connet error"); // 连接失败，打印错误信息并退出程序

    // 设置输入为非阻塞模式
    int flag = fcntl(STDIN_FILENO, F_GETFL);
    flag |= O_NONBLOCK;
    fcntl(STDIN_FILENO, F_SETFL, flag);

    // 设置套接字为非阻塞模式
    flag = fcntl(cfd, F_GETFL);
    flag |= O_NONBLOCK;
    fcntl(cfd, F_SETFL, flag);

    while(1)
    {
        ret = read(STDIN_FILENO, buf, sizeof buf); // 从标准输入读取数据
        if(ret > 0) write(cfd, buf, ret); // 将数据写入套接字
        if(!strncmp(buf,"exit",4)) break; // 如果输入为"exit"，则退出循环
        ret = read(cfd, buf, sizeof buf); // 从套接字读取数据
        if(ret > 0) write(STDOUT_FILENO, buf, ret); // 将数据写入标准输出
    }
    
    close(cfd); // 关闭套接字
    return 0;
}
