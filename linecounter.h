#ifndef LINECOUNTER_H
#define LINECOUNTER_H
#include <fstream>

extern std::string* pLog;

class LineCounter
{
public:
    std::ifstream fin;
    int lineNo;
    std::istream& nextLine(std::string&);
    LineCounter(const std::string&);
    ~LineCounter();
};

#endif // LINECOUNTER_H
