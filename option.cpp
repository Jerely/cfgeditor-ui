#include "option.h"
#include <QString>
#include "defaultdoubleprecision.h"
using namespace std;

string Option::valueToString()
{
    switch(type)
    {
    case BOOL:
        if(get<bool>(value)) {
            return "true";
        }
        return "false";
    case DOUBLE:
        return QString::number(get<double>(value), 'g', DEFAULT_DOUBLE_PRECISION).toUtf8().constData();
    case INT:
        return to_string(get<int64_t>(value));
    case STRING:
        return get<string>(value);
    }
}

string Option::typeToString()
{
    switch(type)
    {
    case BOOL:
        return "bool";
    case DOUBLE:
        return "double";
    case INT:
        return "int";
    case STRING:
        return "string";
    }
}

string Option::toString()
{
    return string(state == UNALTERED ? "" : (state == ALTERED ? "*" : "+")) +
           typeToString() +
           " " + name +
           " = " +
           valueToString();
}

Option::Option()
    : state(UNALTERED)
{}

Option::Option(OptionState state)
    : state(state)
{}

StringToOptionType Option::stringToOptionType =
{
    {"string", STRING},
    {"bool", BOOL},
    {"int", INT},
    {"double", DOUBLE}
};
