#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h> // Windows下的网络编程库

#define MAX_CLIENTS 10
#define MAX_BUFFER_SIZE 1024

void error(const char *msg) {
    perror(msg);
    exit(1);
}

int main() {
    WSADATA wsaData; // 初始化Windows网络库

    // 初始化Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        error("Winsock初始化失败");
    }

    // 创建服务器套接字
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        error("无法创建套接字");
    }

    // 设置服务器地址
    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(8888); // 设置监听端口

    // 绑定套接字
    if (bind(serverSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR) {
        error("绑定套接字失败");
    }

    // 监听连接请求
    if (listen(serverSocket, MAX_CLIENTS) == SOCKET_ERROR) {
        error("监听失败");
    }

    printf("聊天室服务器已启动，等待连接...\n");

    // 处理客户端连接
    struct sockaddr_in clientAddress;
    int clientAddressLength = sizeof(clientAddress);
    int clientSockets[MAX_CLIENTS];
    memset(clientSockets, 0, sizeof(clientSockets));

    while (1) {
        // 接受连接请求
        int newSocket = accept(serverSocket, (struct sockaddr *)&clientAddress, &clientAddressLength);
        if (newSocket == INVALID_SOCKET) {
            error("无法接受连接");
        }

        // 将新连接添加到客户端套接字数组
        int i;
        for (i = 0; i < MAX_CLIENTS; i++) {
            if (clientSockets[i] == 0) {
                clientSockets[i] = newSocket;
                break;
            }
        }

        // 欢迎消息
        char welcomeMessage[MAX_BUFFER_SIZE] = "你已成功连接到聊天室\n";
        send(newSocket, welcomeMessage, strlen(welcomeMessage), 0);

        printf("新连接加入，套接字号：%d，IP：%s，端口：%d\n", newSocket, inet_ntoa(clientAddress.sin_addr), ntohs(clientAddress.sin_port));

        // 创建子进程处理客户端消息
        if (fork() == 0) {
            // 子进程用于接收和转发消息
            close(serverSocket);

            while (1) {
                char buffer[MAX_BUFFER_SIZE];
                memset(buffer, 0, sizeof(buffer));

                // 接收消息
                int bytesRead = recv(newSocket, buffer, sizeof(buffer), 0);
                if (bytesRead <= 0) {
                    // 客户端断开连接
                    printf("套接字号：%d，IP：%s，端口：%d 断开连接\n", newSocket, inet_ntoa(clientAddress.sin_addr), ntohs(clientAddress.sin_port));
                    break;
                }

                // 转发消息给所有连接的客户端
                for (i = 0; i < MAX_CLIENTS; i++) {
                    if (clientSockets[i] != 0 && clientSockets[i] != newSocket) {
                        send(clientSockets[i], buffer, strlen(buffer), 0);
                    }
                }
            }

            // 关闭套接字并退出子进程
            close(newSocket);
            exit(0);
        } else {
            // 父进程继续监听连接请求
            close(newSocket);
        }
    }

    // 关闭服务器套接字并清理Winsock
    closesocket(serverSocket);
    WSACleanup();

    return 0;
}