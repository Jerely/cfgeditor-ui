#include <QFileDialog>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "config.h"
#include <QPushButton>
#include <QHBoxLayout>
#include <experimental/filesystem>
using namespace std;
namespace fs = experimental::filesystem;
const int MIN_DOUBLE_VALUE_SPINBOX_INDEX = 0;
const int MIN_INT_VALUE_SPINBOX_INDEX = 1;
const int MAX_DOUBLE_VALUE_SPINBOX_INDEX = 0;
const int MAX_INT_VALUE_SPINBOX_INDEX = 1;

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
    if(currentRow < 0)
    {
        return;
    }
    Option& option = currentOption();
    ui->optionTypeComboBox->setCurrentIndex(option.type);
    ui->nameLineEdit->setText(option.name.c_str());
    ui->commentTextBox->blockSignals(true);
    ui->commentTextBox->setPlainText(option.comment.c_str());
    ui->commentTextBox->blockSignals(false);
    switch(option.type)
    {
    case BOOL:
        ui->valueStackedWidget->setCurrentIndex(OptionType::BOOL);
        get<bool>(option.value) ? ui->valueCheckBox->setCheckState(Qt::Checked) :
                                  ui->valueCheckBox->setCheckState(Qt::Unchecked);
        break;
    case INT:
        ui->valueStackedWidget->setCurrentIndex(OptionType::INT);
        ui->minStackedWdiget->setCurrentIndex(MIN_INT_VALUE_SPINBOX_INDEX);
        ui->maxStackedWidget->setCurrentIndex(MAX_INT_VALUE_SPINBOX_INDEX);
        ui->valueSpinBox->setValue(get<int64_t>(option.value));
        try {
            ui->minValueSpinBox->setValue(get<int64_t>(option.min));
        } catch (bad_variant_access&) {
            ui->minValueSpinBox->setValue(numeric_limits<int64_t>::min());
        }
        try {
            ui->maxValueSpinBox->setValue(get<int64_t>(option.max));
        } catch (bad_variant_access&) {
            ui->maxValueSpinBox->setValue(numeric_limits<int64_t>::max());
        }
        break;
    case DOUBLE:
        ui->valueStackedWidget->setCurrentIndex(OptionType::DOUBLE);
        ui->minStackedWdiget->setCurrentIndex(MIN_DOUBLE_VALUE_SPINBOX_INDEX);
        ui->maxStackedWidget->setCurrentIndex(MAX_DOUBLE_VALUE_SPINBOX_INDEX);
        ui->doubleSpinBox->setValue(get<double>(option.value));
        try {
            ui->minDoubleSpinBox->setValue(get<double>(option.min));
        } catch (bad_variant_access&) {
            ui->minDoubleSpinBox->setValue(numeric_limits<double>::lowest());
        }
        try {
            ui->maxDoubleSpinBox->setValue(get<double>(option.max));
        } catch (bad_variant_access&) {
            ui->maxDoubleSpinBox->setValue(numeric_limits<double>::max());
        }
        break;
    case STRING:
        ui->valueStackedWidget->setCurrentIndex(OptionType::STRING);
        ui->valueLineEdit->setText(get<string>(option.value).c_str());
        break;
    }
    if(option.type == INT || option.type == DOUBLE)
    {
        ui->minStackedWdiget->show();
        ui->maxStackedWidget->show();
    }
    else
    {
        ui->minStackedWdiget->hide();
        ui->maxStackedWidget->hide();
    }
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
    ui->optionTypeComboBox->insertItem(OptionType::BOOL, "bool");
    ui->optionTypeComboBox->insertItem(OptionType::STRING, "string");
    ui->optionTypeComboBox->insertItem(OptionType::INT, "int");
    ui->optionTypeComboBox->insertItem(OptionType::DOUBLE, "double");
    const int64_t int64min = numeric_limits<int64_t>::min();
    const int64_t int64max = numeric_limits<int64_t>::max();
    ui->valueSpinBox->setMinimum(int64min);
    ui->valueSpinBox->setMaximum(int64max);
    ui->minValueSpinBox->setMinimum(int64min);
    ui->minValueSpinBox->setMaximum(int64max);
    ui->maxValueSpinBox->setMinimum(int64min);
    ui->maxValueSpinBox->setMaximum(int64max);
}

Option &MainWindow::currentOption()
{
    return *configs[static_cast<uint64_t>(ui->tabsWidget->currentIndex())]->options[static_cast<uint64_t>(ui->optionsListWidget->currentRow())];
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
    try {
        if(newValue < get<int64_t>(currentOption().min))
        {
            ui->statusbar->showMessage("Значение типа int меньше указанного мин. значения.");
            ui->valueSpinBox->setValue(get<int64_t>(currentOption().value));
            return;
        }
    } catch (bad_variant_access&) {}
    try {
        if(newValue > get<int64_t>(currentOption().max))
        {
            ui->statusbar->showMessage("Значение типа int превышает указанное макс. значение.");
            ui->valueSpinBox->setValue(get<int64_t>(currentOption().value));
            return;
        }
    } catch (bad_variant_access&) {}
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
        int64_t min;
        try {
            min = get<int64_t>(currentOption().min);
        } catch (bad_variant_access&) {
            min = numeric_limits<int64_t>::min();
        }
        ui->minValueSpinBox->setValue(min);
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
        int64_t max;
        try {
            max = get<int64_t>(currentOption().max);
        } catch (bad_variant_access&) {
            max = numeric_limits<int64_t>::max();
        }
        ui->maxValueSpinBox->setValue(max);
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
    try {
        if(newValue < get<double>(currentOption().min))
        {
            ui->statusbar->showMessage("Значение типа double меньше указанного мин. значения.");
            ui->doubleSpinBox->setValue(get<double>(currentOption().value));
            return;
        }
    } catch (bad_variant_access&) {}
    try {
        if(newValue > get<double>(currentOption().max))
        {
            ui->statusbar->showMessage("Значение типа double превышает указанное макс. значение.");
            ui->doubleSpinBox->setValue(get<double>(currentOption().value));
            return;
        }
    } catch (bad_optional_access&) {}
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
        double min;
        try {
            min = get<double>(currentOption().min);
        } catch (bad_variant_access&) {
            min = numeric_limits<double>::lowest();
        }
        ui->minDoubleSpinBox->setValue(min);
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
        double max;
        try {
            max = get<double>(currentOption().max);
        } catch (bad_variant_access&) {
            max = numeric_limits<double>::max();
        }
        ui->maxDoubleSpinBox->setValue(max);
        return;
    }
    currentOption().max = newMax;
}

void MainWindow::on_valueLineEdit_editingFinished()
{
    if(ui->optionsListWidget->currentRow() < 0)
    {
        return;
    }
    if(ui->valueLineEdit->text().length() > 30000)
    {
        ui->statusbar->showMessage("Длина значения типа string превышает 30000 символов.");
        ui->valueLineEdit->setText(get<string>(currentOption().value).c_str());
        return;
    }
    currentOption().value = string(ui->valueLineEdit->text().toUtf8().constData());
    updateCurItem();
}

void MainWindow::on_commentTextBox_textChanged()
{
    while(ui->commentTextBox->toPlainText().length() > 30) {
        ui->commentTextBox->textCursor().deletePreviousChar();
    }
    currentOption().comment = ui->commentTextBox->toPlainText().toUtf8().constData();
    for(auto& ch : currentOption().comment) {
        if(ch == '\n' || ch == '\r') {
            ch = ' ';
        }
    }
}
