#pragma once

#ifndef UTILS_H
#define UTILS_H
#include <string>
#include <winsock2.h>


bool isInteger(std::string str);
bool isFloat(std::string str);
bool isIntegerStd(std::string str);
bool isFloatStd(std::string str);

void destroySocket(SOCKET& socket);
int initializeSocket(std::string ipAddress, int port, int tout, SOCKET& outSocket);

#endif