#ifndef LOGGER_H
#define LOGGER_H
#include <fstream>


class Logger
{
private:
    std::string fileName;
    std::ofstream fout;
public:
    std::string outputStr;
    bool isTesting;
    Logger(std::string);
    ~Logger();
    void log(const std::string&);
};

#endif // LOGGER_H
