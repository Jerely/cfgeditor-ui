#include "logger.h"
using namespace std;

Logger::Logger(string fileName)
    : fileName(fileName),
      fout(fileName),
      isTesting(false)
{}

Logger::~Logger()
{
    fout.close();
}

void Logger::log(const string & msg)
{
    if(isTesting)
    {
        outputStr.append(msg);
    }
    else
    {
        fout << msg;
    }
}

