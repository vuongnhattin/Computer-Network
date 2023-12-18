#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "Socket.h"
#include <WinSock2.h>
#include <iostream>
#include <WS2tcpip.h>
#include <string>
#include "main.h"
#include <iostream>
#include <sstream>
#include <bitset>

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

// Function to convert IP address from string to numeric representation
unsigned int ipToNumber(const char* ip) {
    unsigned int num = 0;
    std::istringstream iss(ip);
    std::string token;
    while (std::getline(iss, token, '.')) {
        num = (num << 8) | std::stoi(token);
    }
    return num;
}

// Function to check if ip2 is in the same subnet as ip1
bool checkInSubnet(const char* ip1, const char* subnetMask, const char* ip2) {
    unsigned int ip1Num = ipToNumber(ip1);
    unsigned int subnetMaskNum = ipToNumber(subnetMask);
    unsigned int ip2Num = ipToNumber(ip2);

    // Calculate the network addresses for ip1 and ip2
    unsigned int networkIp1 = ip1Num & subnetMaskNum;
    unsigned int networkIp2 = ip2Num & subnetMaskNum;

    // Check if ip2 is in the same subnet as ip1
    return (networkIp1 == networkIp2);
}

void broadcastS() {
    WSAData data;
    if (WSAStartup(MAKEWORD(2, 2), &data) != 0) {
        std::cout << "WSAStartup fail!\n";
        return;
    }
    else {
        std::cout << "WSAStartup ok!\n";
    }
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
    char subnetMask[16];
    std::cout << "waiting for name req from client broadcast.\n";
    while (connectionState != ConnectionState::CONNECTED)
    {
        ZeroMemory(&client, clientLength);
        ZeroMemory(subnetMask, 16);
        int bytesIn = recvfrom(in, subnetMask, 16, 0, (sockaddr*)&client, &clientLength);
        if (bytesIn == SOCKET_ERROR)
        {
            std::cout << "Error receiving from client " << WSAGetLastError() << "\n";
            continue;
        }
        char clientIp[16];
        ZeroMemory(clientIp, 16);
        inet_ntop(AF_INET, &client.sin_addr, clientIp, 16);
        std::cout << "Message recv from " << clientIp << " : " << subnetMask << "\n";
        char computerName[100]{ 0 };
        gethostname(computerName, 100);

        sendto(in, computerName, strlen(computerName) + 1, 0, (sockaddr*)&client, clientLength);

        struct hostent* phe = gethostbyname(computerName);

        for (int i = 0; phe->h_addr_list[i] != 0; ++i) {
            struct in_addr addr;
            memcpy(&addr, phe->h_addr_list[i], sizeof(struct in_addr));
            memcpy(ip, inet_ntoa(addr), 16);
            if (checkInSubnet(clientIp, subnetMask, ip)) {
                discoveryState = DiscoveryState::SUCCESS;
                break;
			}
        }
    }
    closesocket(in);
    std::cout << "shut down broadcast socket.\n";
}

void sendScreenResolution(SOCKET acceptSocket, int width, int height) {
    std::string widthStr = std::to_string(width);
    std::string heightStr = std::to_string(height);
	send(acceptSocket, widthStr.c_str(), 10, 0);
    send(acceptSocket, heightStr.c_str(), 10, 0);
}