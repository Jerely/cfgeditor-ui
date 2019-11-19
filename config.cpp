#include "config.h"
#include <regex>
#include <iostream>
#include <memory>
#include "badsyntaxerror.h"
using namespace std;

Config::Config(string filename, Logger& logger)
    : filename(filename)
      , lineCounter(nullptr)
      , logger(logger)
{
}

void Config::parseConfig()
{
    lineCounter = new LineCounter(filename);
    try {
        parseModuleName();
        parseOptions();
    } catch (BadConfigError& e) {
        prepareMessage(e.what());
    }
    delete lineCounter;
    lineCounter = nullptr;
}

void Config::parseModuleName()
{
    string rawModuleName;
    smatch match;
    if(!lineCounter->nextLine(rawModuleName)
            || !regex_search(rawModuleName, match, Regex::moduleName))
    {
        throw BadConfigError("ожидалось имя модуля");
    }
    if(match[1].str().size() > 30)
    {
        throw BadConfigError("длина имени модуля превышает 30 символов");
    }
    moduleName = match[1];
}

void Config::parseOptions()
{
    string rawComment;
    while(lineCounter->nextLine(rawComment))
    {
        if(rawComment.length() < 3 || rawComment[0] != '#' || rawComment[1] != ' ')
        {
            continue;
        }
        try {
            auto option = make_unique<Option>();
            parseOption(rawComment, *option);
            options.push_back(move(option));
        } catch (BadOptionError& e) {
            prepareMessage(e.what());
        }
    }
}

void Config::parseOption(const string& rawComment, Option& option)
{
    parseComment(rawComment, option.comment);
    option.type = parseType();
    string rawString;
    if(!lineCounter->nextLine(rawString))
    {
        throw BadOptionError("ожидалось имя опции");
    }
    if(regex_match(rawString, Regex::boundaries))
    {
        parseMinAndMax(rawString, option.type, option.min, option.max);
        if(!lineCounter->nextLine(rawString))
        {
            throw BadOptionError("ожидалось имя опции");
        }
    }
    parseNameAndValue(rawString, option.type, option.min, option.max, option.name, option.value);
}

void Config::parseComment(const string& rawComment, string& comment)
{
    if(rawComment.length() < 3 || rawComment[0] != '#' || rawComment[1] != ' ')
    {
        throw BadOptionError("ожидался комментарий");
    }
    if(rawComment.length() > 30002)
    {
        throw BadOptionError("длина комментария превышает 30000 символов");
    }
    comment = rawComment;
    comment.erase(0, 2);
}

OptionType Config::parseType()
{
    string rawType;
    smatch match;
    if(!lineCounter->nextLine(rawType)
            || !regex_search(rawType, match, Regex::type))
    {
        throw BadOptionError("ожидался тип");
    }
    return Option::stringToOptionType[match[1]];
}

void Config::parseMinAndMax(const string& rawMinAndMax,
                            OptionType optionType,
                            OptionMinMax& min,
                            OptionMinMax& max)
{
    smatch match;
    regex_search(rawMinAndMax, match, Regex::boundaries);
    string rawMin = match[1];
    if(rawMin != "")
    {
        if(!regex_match(rawMin, Regex::doubleValue))
        {
            throw BadOptionError("ожидалось минимальное значение");
        }
        if(optionType != DOUBLE && optionType != INT)
        {
            throw BadOptionError("несоответствие мин. значения типу");
        }
        if(optionType == INT && !regex_match(rawMin, Regex::intValue))
        {
            throw BadOptionError("несоответствие мин. значения типу int");
        }
        if(optionType == INT)
        {
            try {
                min = stoll(rawMin);
            } catch (out_of_range&) {
                throw BadOptionError("указанное минимальное значение не входит в допустимый диапазон");
            }
        }
        else
        {
            min = stod(rawMin);
        }
    }
    string rawMax = match[2];
    if(rawMax != "")
    {
        if(!regex_match(rawMax, Regex::doubleValue))
        {
            throw BadOptionError("ожидалось максимальное значение");
        }
        if(optionType != DOUBLE && optionType != INT)
        {
            throw BadOptionError("несоответствие макс. значения типу");
        }
        if(optionType == INT && !regex_match(rawMax, Regex::intValue))
        {
            throw BadOptionError("несоответствие макс. значения типу int");
        }
        if(optionType == INT)
        {
            try {
                max = stoll(rawMax);
            } catch (out_of_range&) {
                throw BadOptionError("указанное максимальное значение не входит в допустимый диапазон");
            }
        }
        else
        {
            max = stod(rawMax);
        }
    }
}

void Config::parseNameAndValue(const string& rawNameAndValue,
                               OptionType optionType,
                               const OptionMinMax& min,
                               const OptionMinMax& max,
                               string& name,
                               OptionValue& value)
{
    istringstream iss(rawNameAndValue);
    vector<string> tokens{istream_iterator<string>{iss}, istream_iterator<string>{}};
    if(tokens.size() < 3)
    {
        throw BadOptionError("ожидалось имя опции и значение");
    }
    if(tokens[0].length() > 30)
    {
        throw BadOptionError("длина имени опции превышает 30 символов");
    }
    if(!regex_match(tokens[0], Regex::optionName))
    {
        throw BadOptionError("ожидалось имя опции");
    }
    name = tokens[0];
    string rawValue = tokens[2];
    switch(optionType)
    {
    case INT:
        if(!regex_match(rawValue, Regex::intValue))
        {
            throw BadOptionError("ожидалось значение типа int");
        }
        try {
            value = stoll(rawValue);
        } catch (out_of_range&) {
            throw BadOptionError("указанное значение не входит в допустимый диапазон");
        }
        if(holds_alternative<int64_t>(min) && get<int64_t>(value) < get<int64_t>(min)) {
            throw BadOptionError("значение не соответствует указанному диапазону");
        }
        if(holds_alternative<int64_t>(max) && get<int64_t>(value) > get<int64_t>(max)) {
            throw BadOptionError("значение не соответствует указанному диапазону");
        }
        break;
    case DOUBLE:
        if(!regex_match(rawValue, Regex::doubleValue))
        {
            throw BadOptionError("значение опции не соответствует типу double");
        }
        value = stod(rawValue);
        if(holds_alternative<double>(min) && get<double>(value) < get<double>(min)) {
            throw BadOptionError("значение не соответствует указанному диапазону");
        }
        if(holds_alternative<double>(max) && get<double>(value) > get<double>(max)) {
            throw BadOptionError("значение не соответствует указанному диапазону");
        }
        break;
    case BOOL:
        if(!regex_match(rawValue, Regex::boolValue))
        {
            throw BadOptionError("ожидалось значение типа bool");
        }
        value = bool(rawValue == "true");
        break;
    case STRING:
        if(rawValue.length() > 30000)
        {
            throw BadOptionError("длина значения опции типа string превышает 30000 символов");
        }
        value = rawValue;
        break;
    }
}

void Config::prepareMessage(const string & reason)
{
    logger.log(string("Синтаксическая ошибка, файл ") +
               filename +
               ", строка " +
               to_string(lineCounter->lineNo) +
               ": " +
               reason +
               ".\n");
}


