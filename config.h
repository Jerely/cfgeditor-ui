#ifndef CONFIG_H
#define CONFIG_H
#include "linecounter.h"
#include "option.h"
#include <deque>
#include <memory>
#include <regex>
#include "logger.h"

namespace Regex
{
    const std::regex moduleName("^###\\s(.*)$");
    const std::regex comment("^#\\s(.*)$");
    const std::regex type("^##--(string|int|double|bool)$");
    const std::regex boundaries("^##([^,]*),([^,]*)$");
    const std::regex intValue("^[+-]?\\d*$");
    const std::regex doubleValue("^[+-]?((\\d+)|(\\d+\\.\\d*)|(\\d*\\.\\d+))([Ee][+-]?\\d+)?$");
    const std::regex optionNameAndValue("^(\\w(\\w|\\d)*)\\s=\\s(.*)$");
    const std::regex optionName("^[a-zA-Z](\\w|\\d)*$");
    const std::regex boolValue("^(false|true)$");
}

class Config
{
    LineCounter* lineCounter;
    Logger& logger;
    void parseModuleName();
    void parseOptions();
    void parseOption(const std::string&, Option&);
    void parseComment(const std::string&, std::string&);
    OptionType parseType();
    void parseMinAndMax(const std::string&, OptionType, OptionMinMax&, OptionMinMax&);
    void parseNameAndValue(const std::string&, OptionType, const OptionMinMax&, const OptionMinMax&, std::string&, OptionValue&);
    inline void prepareMessage(const std::string&);
public:
    std::string filename;
    std::string moduleName;
    std::deque<std::unique_ptr<Option>> options;
    Config(std::string, Logger&);
    void parseConfig();
};

#endif // CONFIG_H
