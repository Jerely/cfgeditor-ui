#include "configtester.h"
#include "config.h"
#include <iostream>
using namespace std;

ConfigTester::ConfigTester(Logger& logger)
    : logger(logger)
{

}


void ConfigTester::typePosTest()
{
    Config config("tests/type-pos.cfg", logger);
    logger.isTesting = false;
    config.parseConfig();
}


void ConfigTester::runAllTests()
{
    basicTest("tests/type-neg.cfg",
              "test 1.1.1., type-neg.cfg",
              "Синтаксическая ошибка, файл tests/type-neg.cfg, строка 3: ожидался тип.\n");
    basicTest("tests/type-pos.cfg",
              "test 1.1.2., type-pos.cfg",
              "");
    basicTest("tests/id-regex-neg.cfg",
              "test 1.2.1., id-regex-neg.cfg",
              "Синтаксическая ошибка, файл tests/id-regex-neg.cfg, строка 4: ожидалось имя опции.\n");
    basicTest("tests/id-len-neg.cfg",
              "test 1.2.2., id-len-neg.cfg",
              "Синтаксическая ошибка, файл tests/id-len-neg.cfg, строка 4: длина имени опции превышает 30 символов.\n");
    basicTest("tests/range-regex-neg.cfg",
              "test 1.3.1.1., range-regex-neg.cfg",
              "Синтаксическая ошибка, файл tests/range-regex-neg.cfg, строка 4: ожидалось минимальное значение.\n");
    basicTest("tests/range-regex-pos.cfg",
              "test 1.3.1.2., range-regex-pos.cfg",
              "");
    basicTest("tests/range-range-neg.cfg",
              "test 1.3.1.3., range-range-neg.cfg",
              "Синтаксическая ошибка, файл tests/range-range-neg.cfg, строка 4: указанное максимальное значение не входит в допустимый диапазон.\n");
    basicTest("tests/range-range-pos.cfg",
              "test 1.3.1.4., range-range-pos.cfg",
              "");
    basicTest("tests/val-range-neg.cfg",
              "test 1.3.2.1., val-range-neg.cfg",
              "Синтаксическая ошибка, файл tests/val-range-neg.cfg, строка 4: указанное значение не входит в допустимый диапазон.\n");
    basicTest("tests/val-range-pos-min.cfg",
              "test 1.3.2.2., val-range-pos-min.cfg",
              "");
    basicTest("tests/val-range-pos-max.cfg",
              "test 1.3.2.2., val-range-pos-max.cfg",
              "");
    basicTest("tests/val-regex-neg.cfg",
              "test 1.3.2.3., val-regex-neg.cfg",
              "Синтаксическая ошибка, файл tests/val-regex-neg.cfg, строка 4: ожидалось значение типа int.\n");
    basicTest("tests/range-neg.cfg",
              "test 1.3.2.5., range-neg.cfg",
              "Синтаксическая ошибка, файл tests/range-neg.cfg, строка 5: значение не соответствует указанному диапазону.\n");
    basicTest("tests/range-pos.cfg",
              "test 1.3.2.6., range-pos.cfg",
              "");
    basicTest("tests/bool-neg.cfg",
              "test 1.3.3.1., bool-neg.cfg",
              "Синтаксическая ошибка, файл tests/bool-neg.cfg, строка 4: ожидалось значение типа bool.\n");
    basicTest("tests/bool-pos.cfg",
              "test 1.3.3.2., bool-pos.cfg",
              "");
    basicTest("tests/string-neg.cfg",
              "test 1.3.4.1., string-neg.cfg",
              "Синтаксическая ошибка, файл tests/string-neg.cfg, строка 4: длина значения опции типа string превышает 30000 символов.\n");
    basicTest("tests/string-pos.cfg",
              "test 1.3.4.2., string-pos.cfg",
              "");
    basicTest("tests/comment-neg.cfg",
              "test 1.4.1., comment-neg.cfg",
              "Синтаксическая ошибка, файл tests/comment-neg.cfg, строка 2: длина комментария превышает 30000 символов.\n");
    basicTest("tests/comment-pos.cfg",
              "test 1.4.2., comment-pos.cfg",
              "");
    basicTest("tests/newline-pos.cfg",
              "test 1.7.2., newline-pos.cfg",
              "");
    basicTest("tests/mod-neg.cfg",
              "test 1.12.1., mod-neg.cfg",
              "Синтаксическая ошибка, файл tests/mod-neg.cfg, строка 1: ожидалось имя модуля.\n");
    basicTest("tests/mod-len-neg.cfg",
              "test 1.12.3., mod-len-neg.cfg",
              "Синтаксическая ошибка, файл tests/mod-len-neg.cfg, строка 1: длина имени модуля превышает 30 символов.\n");
    //seeOutput("tests/mod-len-neg.cfg");
}

void ConfigTester::basicTest(const string& fileName,
                        const string& testName,
                        const string& referenceString)
{
    Config config(fileName, logger);
    logger.outputStr = "";
    logger.isTesting = true;
    config.parseConfig();
    if(logger.outputStr == referenceString)
    {
        cout << "[OK] " << testName << endl;
    }
    else
    {
        cout << "[FAILURE] " << testName << endl;
    }
}

void ConfigTester::seeOutput(const string& fileName)
{
    Config config(fileName, logger);
    logger.isTesting = false;
    config.parseConfig();
}
