#include "logger.h"
#include <fstream>
#include <filesystem>
using namespace std;
namespace fs = filesystem;

Logger::Logger(string fileName)
    : fileName(fileName)
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
    fout << msg;
    fout.close();
}

