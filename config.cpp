#include <QString>
#include "config.h"
#include <regex>
#include <iostream>
#include <fstream>
#include <memory>
#include "badsyntaxerror.h"
using namespace std;

Config::Config(string filename, Logger& logger)
    : logger(logger)
    , lineNo(0)
    , isAltered(false)
    , path(filename)
    , parseError(false)
{}

Config::~Config() {}

bool Config::parseConfig()
{
    return parseConfig(path);
}

bool Config::parseConfig(const string &filename)
{
    fin = make_unique<ifstream>(filename);
    try {
        parseModuleName();
        parseOptions();
    } catch (BadConfigError& e) {
        prepareMessage(e.what());
        fin->close();
        return false;
    }
    fin->close();
    return true;
}

istream &Config::nextLine(string &outl)
{
    istream& i = getline(*fin, outl);
    if(outl[outl.length()-1] == '\r') {
        outl.erase(outl.length()-1);
    }
    if(i) {
        lineNo++;
    }
    return i;
}

void Config::parseModuleName()
{
    string rawModuleName;
    smatch match;
    if(!nextLine(rawModuleName)
            || !regex_search(rawModuleName, match, Regex::moduleName)) {
        parseError = true; throw BadConfigError("ожидалось имя модуля");
    }
    if(match[1].str().size() > 30) {
        parseError = true; throw BadConfigError("длина имени модуля превышает 30 символов");
    }
    moduleName = match[1];
}

void Config::parseOptions()
{
    string rawComment;
    while(nextLine(rawComment))
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
    if(!nextLine(rawString))
    {
        parseError = true; throw BadOptionError("ожидалось имя опции");
    }
    if(regex_match(rawString, Regex::boundaries))
    {
        parseMinAndMax(rawString, option.type, option.min, option.max);
        if(!nextLine(rawString))
        {
            parseError = true; throw BadOptionError("ожидалось имя опции");
        }
    }
    parseNameAndValue(rawString, option.type, option.min, option.max, option.name, option.value);
}

void Config::parseComment(const string& rawComment, string& comment)
{
    if(rawComment.length() < 3 || rawComment[0] != '#' || rawComment[1] != ' ')
    {
        parseError = true; throw BadOptionError("ожидался комментарий");
    }
    if(rawComment.length() > 30002)
    {
        parseError = true; throw BadOptionError("длина комментария превышает 30000 символов");
    }
    comment = rawComment;
    comment.erase(0, 2);
}

OptionType Config::parseType()
{
    string rawType;
    smatch match;
    if(!nextLine(rawType)
            || !regex_search(rawType, match, Regex::type))
    {
        parseError = true; throw BadOptionError("ожидался тип");
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
            parseError = true; throw BadOptionError("ожидалось минимальное значение");
        }
        if(optionType != DOUBLE && optionType != INT)
        {
            parseError = true; throw BadOptionError("несоответствие мин. значения типу");
        }
        if(optionType == INT && !regex_match(rawMin, Regex::intValue))
        {
            parseError = true; throw BadOptionError("несоответствие мин. значения типу int");
        }
        if(optionType == INT)
        {
            try {
                min = stol(rawMin);
            } catch (out_of_range&) {
                parseError = true; throw BadOptionError("указанное минимальное значение не входит в допустимый диапазон");
            }
        }
        else
        {
            if(rawMin.length() > 300) {
                parseError = true; throw BadOptionError("длина мин. значения типа double превышает 300 символов");
            }
            min = QString(rawMin.c_str()).toDouble();
        }
    }
    string rawMax = match[2];
    if(rawMax != "")
    {
        if(!regex_match(rawMax, Regex::doubleValue))
        {
            parseError = true; throw BadOptionError("ожидалось максимальное значение");
        }
        if(optionType != DOUBLE && optionType != INT)
        {
            parseError = true; throw BadOptionError("несоответствие макс. значения типу");
        }
        if(optionType == INT && !regex_match(rawMax, Regex::intValue))
        {
            parseError = true; throw BadOptionError("несоответствие макс. значения типу int");
        }
        if(optionType == INT) {
            try {
                max = stol(rawMax);
            } catch (out_of_range&) {
                parseError = true; throw BadOptionError("указанное максимальное значение не входит в допустимый диапазон");
            }
        }
        else {
            if(rawMax.length() > 300) {
                parseError = true; throw BadOptionError("длина макс. значения типа double превышает 300 символов");
            }
            max = QString(rawMax.c_str()).toDouble();
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
        parseError = true; throw BadOptionError("ожидалось имя опции и значение");
    }
    if(tokens[0].length() > 30)
    {
        parseError = true; throw BadOptionError("длина имени опции превышает 30 символов");
    }
    if(!regex_match(tokens[0], Regex::optionName))
    {
        parseError = true; throw BadOptionError("ожидалось имя опции");
    }
    name = tokens[0];
    string rawValue = tokens[2];
    switch(optionType)
    {
    case INT:
        if(!regex_match(rawValue, Regex::intValue))
        {
            parseError = true; throw BadOptionError("ожидалось значение типа int");
        }
        try {
            value = stol(rawValue);
        } catch (out_of_range&) {
            parseError = true; throw BadOptionError("указанное значение не входит в допустимый диапазон");
        }
        if(holds_alternative<int64_t>(min) && get<int64_t>(value) < get<int64_t>(min)) {
            parseError = true; throw BadOptionError("значение не соответствует указанному диапазону");
        }
        if(holds_alternative<int64_t>(max) && get<int64_t>(value) > get<int64_t>(max)) {
            parseError = true; throw BadOptionError("значение не соответствует указанному диапазону");
        }
        break;
    case DOUBLE:
        if(!regex_match(rawValue, Regex::doubleValue))
        {
            parseError = true; throw BadOptionError("значение опции не соответствует типу double");
        }
        if(rawValue.length() > 300) {
            parseError = true; throw BadOptionError("длина значения опции типа double превышает 300 символов");
        }
        value = QString(rawValue.c_str()).toDouble();
        if(holds_alternative<double>(min) && get<double>(value) < get<double>(min)) {
            parseError = true; throw BadOptionError("значение не соответствует указанному диапазону");
        }
        if(holds_alternative<double>(max) && get<double>(value) > get<double>(max)) {
            parseError = true; throw BadOptionError("значение не соответствует указанному диапазону");
        }
        break;
    case BOOL:
        if(!regex_match(rawValue, Regex::boolValue))
        {
            parseError = true; throw BadOptionError("ожидалось значение типа bool");
        }
        value = bool(rawValue == "true");
        break;
    case STRING:
        if(rawValue.length() > 30000)
        {
            parseError = true; throw BadOptionError("длина значения опции типа string превышает 30000 символов");
        }
        value = rawValue;
        break;
    }
}

void Config::prepareMessage(const string & reason)
{
    logger.log(string("Синтаксическая ошибка, файл ") +
               path +
               ", строка " +
               to_string(lineNo) +
               ": " +
               reason +
               ".\n");
}


