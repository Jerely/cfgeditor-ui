#include "badsyntaxerror.h"
using namespace std;

BadSyntaxError::BadSyntaxError(const string& reason)
    : reason(reason)
{
}

const char* BadSyntaxError::what() const noexcept
{
   return reason.c_str();
}


BadOptionError::BadOptionError(const string& reason)
    : BadSyntaxError(reason)
{}

BadConfigError::BadConfigError(const string & reason)
    : BadSyntaxError(reason)
{}
