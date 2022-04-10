#ifndef UTILS_CPP
#define UTILS_CPP

#include <iostream>
#include <sstream>



static bool isInteger(std::string str) {
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

static bool isFloat(std::string str) {
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

static bool isIntegerStd(std::string str)
{
    for (char const& c : str) {
        if (std::isdigit(c) == 0) return false;
    }
    return true;
}

static bool isFloatStd(std::string str)
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


#endif // UTILS_CPP