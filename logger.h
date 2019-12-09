#ifndef LOGGER_H
#define LOGGER_H
#include <string>


class Logger
{
private:
    std::string fileName;
public:
    Logger(std::string);
    void log(const std::string&);
};

#endif // LOGGER_H
