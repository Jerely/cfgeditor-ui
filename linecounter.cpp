#include "linecounter.h"
using namespace std;

LineCounter::LineCounter(const string& pathToFile)
    :
      fin(pathToFile),
      lineNo(0)
{}

LineCounter::~LineCounter()
{
    fin.close();
}

istream& LineCounter::nextLine(string& outl)
{
    istream& i = getline(fin, outl);
    if(i)
    {
        lineNo++;
    }
    return i;
}
