#ifndef MMCONFIG_H
#define MMCONFIG_H

#include <string>

struct MMConfig
{
    int ival;
    std::string istr;

    int readFromFile(const std::string &filepath);
    int writeFile(const std::string &filepath);
};

#endif