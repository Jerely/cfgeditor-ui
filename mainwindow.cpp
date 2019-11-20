#include <QFileDialog>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "config.h"
#include <QPushButton>
#include <QHBoxLayout>
#include <experimental/filesystem>
using namespace std;
namespace fs = experimental::filesystem;

const int MAX_STR_LEN = 30000;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , logger("log.txt")
{
    ui->setupUi(this);
    customSetup();
    string projDir;
    //configs.push_back(make_unique<Config>("tests\\newline-pos.cfg", logger));
    //configs.push_back(make_unique<Config>("tests\\sample-options.cfg", logger));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_optionsListWidget_currentRowChanged(int currentRow)
{
    if(currentRow < 0) {
        return;
    }
    updateInfo();
}

void MainWindow::on_nameLineEdit_editingFinished()
{
    if(ui->optionsListWidget->currentRow() < 0)
    {
        return;
    }
    string newName = ui->nameLineEdit->text().toUtf8().constData();
    if(!regex_match(newName, Regex::optionName))
    {
        ui->statusbar->showMessage("Некорректное имя опции.");
        ui->nameLineEdit->setText(currentOption().name.c_str());
        return;
    }
    currentOption().name = newName;
    ui->optionsListWidget->currentItem()->setText(currentOption().toString().c_str());

}

inline void MainWindow::customSetup()
{
    const int64_t int64min = numeric_limits<int64_t>::min();
    const int64_t int64max = numeric_limits<int64_t>::max();
    ui->valueSpinBox->setMinimum(int64min);
    ui->valueSpinBox->setMaximum(int64max);
    ui->minValueSpinBox->setMinimum(int64min);
    ui->minValueSpinBox->setMaximum(int64max);
    ui->maxValueSpinBox->setMinimum(int64min);
    ui->maxValueSpinBox->setMaximum(int64max);
}

inline Option &MainWindow::currentOption()
{
    return *configs[static_cast<uint64_t>(ui->tabsWidget->currentIndex())]->options[static_cast<uint64_t>(ui->optionsListWidget->currentRow())];
}

inline Config &MainWindow::currentConfig()
{
    return *configs[static_cast<uint64_t>(ui->tabsWidget->currentIndex())];
}

void MainWindow::openProjDir(const string &projDir)
{
    configs.clear();
    for (const auto& entry : fs::recursive_directory_iterator(projDir))
    {
        const auto& path = entry.path().u8string();
        if (path.substr(path.find_last_of(".") + 1) == "cfg")
        {
            configs.push_back(make_unique<Config>(path, logger));
        }
    }
    ui->tabsWidget->clear();
    for(uint64_t i = 0; i < configs.size(); ++i)
    {
        configs[i]->parseConfig();
        ui->tabsWidget->addTab(new QWidget, configs[i]->moduleName.c_str());
    }
}

inline void MainWindow::updateCurItem()
{
    ui->optionsListWidget->currentItem()->setText(currentOption().toString().c_str());
}

void MainWindow::onTextChanged(const QPlainTextEdit* plainTextEdit, string& toChange)
{
    while(plainTextEdit->toPlainText().length() > MAX_STR_LEN) {
        plainTextEdit->textCursor().deletePreviousChar();
    }
    toChange = plainTextEdit->toPlainText().toUtf8().constData();
    for(auto& ch : toChange) {
        if(ch == '\n' || ch == '\r') {
            ch = ' ';
        }
    }
}

void MainWindow::updateInfo()
{
    ui->optionTypeComboBox->setCurrentIndex(currentOption().type);
    ui->nameLineEdit->setText(currentOption().name.c_str());
    ui->commentTextBox->blockSignals(true);
    ui->commentTextBox->setPlainText(currentOption().comment.c_str());
    ui->commentTextBox->blockSignals(false);
    switch(currentOption().type)
    {
    case BOOL:
        ui->valueStackedWidget->setCurrentIndex(OptionType::BOOL);
        get<bool>(currentOption().value) ? ui->valueCheckBox->setCheckState(Qt::Checked) :
                                  ui->valueCheckBox->setCheckState(Qt::Unchecked);
        break;
    case INT:
        ui->valueStackedWidget->setCurrentIndex(OptionType::INT);
        ui->valueSpinBox->setValue(get<int64_t>(currentOption().value));
        if(holds_alternative<int64_t>(currentOption().min)) {
            ui->minValueSpinBox->setValue(get<int64_t>(currentOption().min));
        }
        else {
            ui->minValueSpinBox->setValue(numeric_limits<int64_t>::min());
        }
        if(holds_alternative<int64_t>(currentOption().max)) {
            ui->maxValueSpinBox->setValue(get<int64_t>(currentOption().max));
        }
        else {
            ui->maxValueSpinBox->setValue(numeric_limits<int64_t>::max());
        }
        break;
    case DOUBLE:
        ui->valueStackedWidget->setCurrentIndex(OptionType::DOUBLE);
        ui->doubleSpinBox->setValue(get<double>(currentOption().value));
        if(holds_alternative<double>(currentOption().min)) {
            ui->minDoubleSpinBox->setValue(get<double>(currentOption().min));
        }
        else {
            ui->minDoubleSpinBox->setValue(numeric_limits<double>::lowest());
        }
        if(holds_alternative<double>(currentOption().max)) {
            ui->maxDoubleSpinBox->setValue(get<double>(currentOption().max));
        }
        else {
            ui->maxDoubleSpinBox->setValue(numeric_limits<double>::max());
        }
        break;
    case STRING:
        ui->valueStackedWidget->setCurrentIndex(OptionType::STRING);
        ui->valueTextEdit->blockSignals(true);
        ui->valueTextEdit->setPlainText(get<string>(currentOption().value).c_str());
        ui->valueTextEdit->blockSignals(false);
        break;
    }
}

void MainWindow::on_tabsWidget_currentChanged(int index)
{
    if(index < 0)
    {
        return;
    }
    ui->optionsListWidget->clear();
    Config& config = *configs[static_cast<uint64_t>(index)];
    for(uint64_t i = 0; i < config.options.size(); ++i)
    {
       Option& option = *config.options[i];
        QListWidgetItem *newItem = new QListWidgetItem;
        newItem->setText(option.toString().c_str());
        ui->optionsListWidget->insertItem(static_cast<int>(i), newItem);
    }
    ui->optionsListWidget->setCurrentRow(0);
}

void MainWindow::on_projDirLineEdit_editingFinished()
{
    string projDir = ui->projDirLineEdit->text().toUtf8().constData();
    if(projDir == "")
    {
        return;
    }
    openProjDir(projDir);
}

void MainWindow::on_openPushButton_clicked()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
                                                 "~",
                                                 QFileDialog::ShowDirsOnly
                                                 | QFileDialog::DontResolveSymlinks);
    if(dir == "")
    {
        return;
    }
    ui->projDirLineEdit->setText(dir);
    openProjDir(dir.toUtf8().constData());
}

void MainWindow::on_valueSpinBox_editingFinished()
{
    int currentRow = ui->optionsListWidget->currentRow();
    if(currentRow < 0)
    {
        return;
    }
    int64_t newValue = ui->valueSpinBox->value();
    if(holds_alternative<int64_t>(currentOption().min) && newValue < get<int64_t>(currentOption().min))
    {
        ui->statusbar->showMessage("Значение типа int меньше указанного мин. значения.");
        ui->valueSpinBox->setValue(get<int64_t>(currentOption().value));
        return;
    }
    if(holds_alternative<int64_t>(currentOption().max) && newValue > get<int64_t>(currentOption().max))
    {
        ui->statusbar->showMessage("Значение типа int превышает указанное макс. значение.");
        ui->valueSpinBox->setValue(get<int64_t>(currentOption().value));
        return;
    }
    currentOption().value = newValue;
    updateCurItem();
}

void MainWindow::on_minValueSpinBox_editingFinished()
{
    int currentRow = ui->optionsListWidget->currentRow();
    if(currentRow < 0)
    {
        return;
    }
    int64_t newMin = ui->minValueSpinBox->value();
    if(newMin > get<int64_t>(currentOption().value))
    {
        ui->statusbar->showMessage("Мин. значение не может превышать значения опции.");
        if(holds_alternative<int64_t>(currentOption().min)) {
            ui->minValueSpinBox->setValue(get<int64_t>(currentOption().min));
        }
        else {
            ui->minValueSpinBox->setValue(numeric_limits<int64_t>::min());
        }
        return;
    }
    currentOption().min = newMin;
}

void MainWindow::on_maxValueSpinBox_editingFinished()
{
    int currentRow = ui->optionsListWidget->currentRow();
    if(currentRow < 0)
    {
        return;
    }
    int64_t newMax = ui->maxValueSpinBox->value();
    if(newMax < get<int64_t>(currentOption().value))
    {
        ui->statusbar->showMessage("Макс. значение не может быть меньше значения опции.");
        if(holds_alternative<int64_t>(currentOption().max)) {
            ui->maxValueSpinBox->setValue(get<int64_t>(currentOption().max));
        }
        else {
           ui->maxValueSpinBox->setValue(numeric_limits<int64_t>::max());
        }
        return;
    }
    currentOption().max = newMax;
}

void MainWindow::on_doubleSpinBox_editingFinished()
{
    int currentRow = ui->optionsListWidget->currentRow();
    if(currentRow < 0)
    {
        return;
    }
    double newValue = ui->doubleSpinBox->value();
    if(holds_alternative<double>(currentOption().min) &&  newValue < get<double>(currentOption().min))
    {
        ui->statusbar->showMessage("Значение типа double меньше указанного мин. значения.");
        ui->doubleSpinBox->setValue(get<double>(currentOption().value));
        return;
    }
    if(holds_alternative<double>(currentOption().max) && newValue > get<double>(currentOption().max))
    {
        ui->statusbar->showMessage("Значение типа double превышает указанное макс. значение.");
        ui->doubleSpinBox->setValue(get<double>(currentOption().value));
        return;
    }
    currentOption().value = newValue;
    updateCurItem();
}

void MainWindow::on_minDoubleSpinBox_editingFinished()
{
    int currentRow = ui->optionsListWidget->currentRow();
    if(currentRow < 0)
    {
        return;
    }
    double newMin = ui->minDoubleSpinBox->value();
    if(newMin > get<double>(currentOption().value))
    {
        ui->statusbar->showMessage("Мин. значение не может превышать значения опции.");
        if(holds_alternative<double>(currentOption().min)) {
            ui->minDoubleSpinBox->setValue(get<double>(currentOption().min));
        }
        else {
           ui->minDoubleSpinBox->setValue(numeric_limits<double>::lowest());
        }
        return;
    }
    currentOption().min = newMin;
}

void MainWindow::on_maxDoubleSpinBox_editingFinished()
{
    int currentRow = ui->optionsListWidget->currentRow();
    if(currentRow < 0)
    {
        return;
    }
    double newMax = ui->maxDoubleSpinBox->value();
    if(newMax < get<double>(currentOption().value))
    {
        ui->statusbar->showMessage("Макс. значение не может быть меньше значения опции.");
        if(holds_alternative<double>(currentOption().max)) {
            ui->maxDoubleSpinBox->setValue(get<double>(currentOption().max));
        }
        else {
            ui->maxDoubleSpinBox->setValue(numeric_limits<double>::max());
        }
        return;
    }
    currentOption().max = newMax;
}

void MainWindow::on_commentTextBox_textChanged()
{
    onTextChanged(ui->commentTextBox, currentOption().comment);
}

void MainWindow::on_valueTextEdit_textChanged()
{
    string strValue;
    onTextChanged(ui->valueTextEdit, strValue);
    currentOption().value = strValue;
    updateCurItem();
}

void MainWindow::on_optionTypeComboBox_currentIndexChanged(int index)
{
    if(index == OptionType::BOOL) {
        switch(currentOption().type) {
        case BOOL:
            break;
        case INT:
            currentOption().value = static_cast<bool>(get<int64_t>(currentOption().value));
            currentOption().min = monostate();
            currentOption().max = monostate();
            break;
        case STRING:
            currentOption().value = false;
            break;
        case DOUBLE:
            currentOption().value = static_cast<bool>(get<double>(currentOption().value));
            currentOption().min = monostate();
            currentOption().max = monostate();
            break;
        }
    }
    else if(index == OptionType::INT) {
        switch(currentOption().type) {
        case BOOL:
            currentOption().value = static_cast<int64_t>(get<bool>(currentOption().value));
            break;
        case INT:
            break;
        case STRING:
            currentOption().value = 0ll;
            break;
        case DOUBLE:
            currentOption().value = static_cast<int64_t>(get<double>(currentOption().value));
            if(holds_alternative<double>(currentOption().min)) {
                currentOption().min = static_cast<int64_t>(get<double>(currentOption().min));
            }
            if(holds_alternative<double>(currentOption().max)) {
                currentOption().max = static_cast<int64_t>(get<double>(currentOption().max));
            }
            break;
        }
    }
    else if(index == OptionType::STRING) {
        switch(currentOption().type) {
        case BOOL:
            currentOption().value = get<bool>(currentOption().value) ? string("true") : string("false");
            break;
        case INT:
            currentOption().value = to_string(get<int64_t>(currentOption().value));
            currentOption().min = monostate();
            currentOption().max = monostate();
            break;
        case STRING:
            break;
        case DOUBLE:
            currentOption().value = to_string(get<double>(currentOption().value));
            currentOption().min = monostate();
            currentOption().max = monostate();
            break;
        }
    }
    else if(index == OptionType::DOUBLE) {
        switch(currentOption().type) {
        case BOOL:
            currentOption().value = get<bool>(currentOption().value);
            break;
        case INT:
            currentOption().value = get<int64_t>(currentOption().value);
            if(holds_alternative<int64_t>(currentOption().min)) {
                currentOption().min = static_cast<double>(get<int64_t>(currentOption().min));
            }
            if(holds_alternative<int64_t>(currentOption().max)) {
                currentOption().max = static_cast<double>(get<int64_t>(currentOption().max));
            }
            break;
        case STRING:
            currentOption().value = .0;
            break;
        case DOUBLE:
            break;
        }
    }
    currentOption().type = static_cast<OptionType>(index);
    updateInfo();
    updateCurItem();
}

void MainWindow::on_saveButton_clicked()
{
    ofstream fout(currentConfig().filename);
    fout << "### " << currentConfig().moduleName << endl;
    for(const auto& option : currentConfig().options) {
        fout << "# " << option->comment << endl;
        fout << "##--" << option->typeToString() << endl;
        bool thereIsMinOrMax = !holds_alternative<monostate>(option->min) || !holds_alternative<monostate>(option->max);
        if(thereIsMinOrMax) {
            fout << "##";
        }
        if(!holds_alternative<monostate>(option->min)) {
            switch(option->type) {
            case INT:
                fout << get<int64_t>(option->min);
                break;
            case DOUBLE:
                fout << get<double>(option->min);
                break;
            default:
                break;
            }
        }
        if(thereIsMinOrMax) {
            fout << ",";
        }
        if(!holds_alternative<monostate>(option->max)) {
            switch(option->type) {
            case INT:
                fout << get<int64_t>(option->max);
                break;
            case DOUBLE:
                fout << get<double>(option->max);
                break;
            default:
                break;
            }
        }
        if(thereIsMinOrMax) {
            fout << endl;
        }
        fout << option->name << " = " << option->valueToString() << endl;
    }
    fout.close();
}
