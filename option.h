#ifndef OPTION_H
#define OPTION_H
#include <string>
#include <variant>
#include <map>

enum OptionType
{

    BOOL, STRING, INT, DOUBLE
};

typedef std::variant<int64_t, double, std::string, bool> OptionValue;
typedef std::variant<std::monostate, int64_t, double> OptionMinMax;
typedef std::map<std::string, OptionType> StringToOptionType;

class Option
{
public:
    static StringToOptionType stringToOptionType;
    std::string comment;
    OptionType type;
    OptionMinMax min, max;
    std::string name;
    OptionValue value;
    std::string toString();
    std::string valueToString();
    std::string typeToString();
    Option();
};

#endif // OPTION_H
