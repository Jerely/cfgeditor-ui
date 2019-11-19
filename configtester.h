#ifndef CONFIGTESTER_H
#define CONFIGTESTER_H
#include "logger.h"


class ConfigTester
{
    Logger& logger;
    void basicTest(const std::string&, const std::string&, const std::string&);
    void seeOutput(const std::string&);
    void typePosTest();
public:
    ConfigTester(Logger&);
    void runAllTests();
};

#endif // CONFIGTESTER_H
