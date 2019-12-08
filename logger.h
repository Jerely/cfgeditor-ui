#ifndef LOGGER_H
#define LOGGER_H
#include <string>


class Logger
{
private:
    std::string fileName;
public:
    std::string outputStr;
    bool isTesting;
    Logger(std::string);
    void log(const std::string&);
};

#endif // LOGGER_H
