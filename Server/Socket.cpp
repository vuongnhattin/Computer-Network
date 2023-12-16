#include "Socket.h"
#include <WinSock2.h>
#include <iostream>
#include <WS2tcpip.h>
#include <string>
#include "main.h"

bool bindSocket(SOCKET& imageSocket, const char* ip, const int port) {
    WSAData data;
    if (WSAStartup(MAKEWORD(2, 2), &data) != 0) {
        std::cout << "WSAStartup fail!\n";
        return false;
    }
    else {
        std::cout << "WSAStartup ok!\n";
    }

    imageSocket = INVALID_SOCKET;
    imageSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (imageSocket == INVALID_SOCKET) {
        std::cout << "Error at socket()!\n";
        WSACleanup();
        return false;
    }
    else {
        std::cout << "socket() is ok!\n";
    }

    sockaddr_in service;
    service.sin_family = AF_INET;
    if (inet_pton(AF_INET, (const char*)ip, &service.sin_addr.s_addr) != 1) {
        std::cout << "invalid ip address!\n";
        closesocket(imageSocket);
        WSACleanup();
        return false;
    }
    service.sin_port = htons((u_short)port);

    if (bind(imageSocket, (sockaddr*)&service, sizeof(service)) == SOCKET_ERROR) {
        std::cout << "bind() faild!\n";
        closesocket(imageSocket);
        WSACleanup();
        return false;
    }
    else {
        std::cout << "bind() is ok!\n";
    }

    return true;
}

bool listenSocket(SOCKET& serverSocket, SOCKET& acceptSocket) {
    if (listen(serverSocket, 1) == SOCKET_ERROR) {
        std::cout << "error at listen()\n";
        WSACleanup();
        return false;
    }
    else {
        std::cout << "listen() ok!\n";
        std::cout << "listening...\n";
    }

    acceptSocket = accept(serverSocket, NULL, NULL);
    if (acceptSocket == INVALID_SOCKET) {
        std::cout << "failed at accept()\n";
        WSACleanup();
        exit(-1);
    }
    else {
		std::cout << "connected!\n";
	}

    return true;
}

bool initServerSocket(SOCKET& serverSocket, SOCKET& acceptSocket, const char* ip, const int port) {
    return (bindSocket(serverSocket, ip, port) && listenSocket(serverSocket, acceptSocket));
}

void broadcastS() {
    SOCKET in = socket(AF_INET, SOCK_DGRAM, 0);
    sockaddr_in serverHint;
    ZeroMemory(&serverHint, sizeof(serverHint));
    serverHint.sin_addr.S_un.S_addr = ADDR_ANY;
    serverHint.sin_family = AF_INET;
    serverHint.sin_port = htons(broadcastPort);
    if (bind(in, (sockaddr*)&serverHint, sizeof(serverHint)) == SOCKET_ERROR)
    {
        std::cout << "Can't bind socket! " << WSAGetLastError() << "\n";
        return;
    }
    sockaddr_in client;
    int clientLength = sizeof(client);
    char buf[1024];
    std::cout << "waiting for name req from client broadcast.\n";
    while (connectionState != ConnectionState::CONNECTED)
    {
        ZeroMemory(&client, clientLength);
        ZeroMemory(buf, 1024);
        int bytesIn = recvfrom(in, buf, 1024, 0, (sockaddr*)&client, &clientLength);
        if (bytesIn == SOCKET_ERROR)
        {
            std::cout << "Error receiving from client " << WSAGetLastError() << "\n";
            continue;
        }
        char clientIp[256];
        ZeroMemory(clientIp, 256);
        inet_ntop(AF_INET, &client.sin_addr, clientIp, 256);
        std::cout << "Message recv from " << clientIp << " : " << buf << "\n";
        char computerName[100]{ 0 };
        gethostname(computerName, 100);
        sendto(in, computerName, strlen(computerName) + 1, 0, (sockaddr*)&client, clientLength);
    }
    closesocket(in);
}

void sendScreenResolution(SOCKET acceptSocket, int width, int height) {
    std::string widthStr = std::to_string(width);
    std::string heightStr = std::to_string(height);

	send(acceptSocket, widthStr.c_str(), 10, 0);
    send(acceptSocket, heightStr.c_str(), 10, 0);
}