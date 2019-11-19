#ifndef BADSYNTAXERROR_H
#define BADSYNTAXERROR_H
#include <exception>
#include <string>


class BadSyntaxError : public std::exception
{
    std::string reason;
public:
    BadSyntaxError(const std::string&);
    const char* what() const noexcept;
};

class BadOptionError : public BadSyntaxError
{
public:
    BadOptionError(const std::string&);
};
class BadConfigError : public BadSyntaxError
{
public:
    BadConfigError(const std::string&);
};
#endif // BADSYNTAXERROR_H
