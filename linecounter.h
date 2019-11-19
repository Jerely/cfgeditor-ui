#ifndef LINECOUNTER_H
#define LINECOUNTER_H
#include <fstream>

extern std::string* pLog;

class LineCounter
{
    std::ifstream fin;
public:
    int lineNo;
    std::istream& nextLine(std::string&);
    LineCounter(const std::string&);
    ~LineCounter();
};

#endif // LINECOUNTER_H
