#include "logger.h"
#include <fstream>
#include <filesystem>
using namespace std;
namespace fs = filesystem;

Logger::Logger(string fileName)
    : fileName(fileName),
      isTesting(false)
{}

void Logger::log(const string & msg)
{
    ofstream fout;
    if (fs::exists(fileName)) {
        fout.open(fileName, ios_base::app);
    }
    else {
        fout.open(fileName);
    }
    if(isTesting) {
        outputStr.append(msg);
    }
    else {
        fout << msg;
    }
    fout.close();
}

