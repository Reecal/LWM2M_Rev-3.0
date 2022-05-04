#include "Utils.h"

#include <iostream>
#include <sstream>
#include <WS2tcpip.h>



bool isInteger(std::string str) {
    if (str == "") {
        return false;
    }
    int length = str.length();
    if (length == 0) {
        return false;
    }
    int i = 0;
    if (str[0] == '-') {
        if (length == 1) {
            return false;
        }
        i = 1;
    }
    for (; i < length; i++) {
        char c = str[i];
        if (c < '0' || c > '9') {
            return false;
        }
    }
    return true;
}

bool isFloat(std::string str) {
    if (str == "") {
        return false;
    }
    int length = str.length();
    if (length == 0) {
        return false;
    }
    int i = 0;
    if (str[0] == '-') {
        if (length == 1) {
            return false;
        }
        i = 1;
    }
    for (; i < length; i++) {
        char c = str[i];
        if ((c < '0' || c > '9') && c != '.') {
            return false;
        }
    }
    return true;
}

bool isIntegerStd(std::string str)
{
    for (char const& c : str) {
        if (std::isdigit(c) == 0) return false;
    }
    return true;
}

bool isFloatStd(std::string str)
{
    for (char const& c : str) {
        if (!std::isdigit(c))
        {
            if (c != '.')
                return false;
        }
    }
    return true;
}

void destroySocket(SOCKET& socket)
{
    closesocket(socket);
    WSACleanup();
}


int initializeSocket(std::string ipAddress, int port, int tout, SOCKET& outSocket)
{
    WSAData data;
    WORD ver = MAKEWORD(2, 2);
    int wsResult = WSAStartup(ver, &data);
    if (wsResult != 0) {
        std::cerr << "ERR" << wsResult << std::endl;
        return 0;
    }

    outSocket = socket(AF_INET, SOCK_DGRAM, 0);

    if (outSocket == INVALID_SOCKET) {
        std::cerr << "Cannot create socket #" << WSAGetLastError << std::endl;
        WSACleanup();
        return 0;
    }

    DWORD timeout = tout * 1000;
    u_long mode = 1;
    ioctlsocket(outSocket, FIONBIO, &mode);
    //setsockopt(outSocket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof timeout);

    sockaddr_in hint;
    hint.sin_family = AF_INET;
    hint.sin_port = htons(port);
    inet_pton(AF_INET, ipAddress.c_str(), &hint.sin_addr);

    int connResult = connect(outSocket, (sockaddr*)&hint, sizeof(hint));
    if (connResult == SOCKET_ERROR) {
        std::cerr << "Error " << WSAGetLastError << std::endl;
        destroySocket(outSocket);
        return 0;
    }
    return 1;
}

