#include <QMessageBox>
#include <QFileDialog>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "config.h"
#include <QPushButton>
#include <QHBoxLayout>
#include <iostream>
#include <experimental/filesystem>
using namespace std;
namespace fs = experimental::filesystem;

#define BLOCK_UNWANTED_SIGNALS() \
    const QSignalBlocker blocker1(ui->valueSpinBox); \
    const QSignalBlocker blocker2(ui->valueCheckBox); \
    const QSignalBlocker blocker3(ui->valueTextEdit); \
    const QSignalBlocker blocker4(ui->minSpinBox); \
    const QSignalBlocker blocker5(ui->maxSpinBox); \
    const QSignalBlocker blocker6(ui->minDoubleSpinBox); \
    const QSignalBlocker blocker7(ui->maxDoubleSpinBox); \
    const QSignalBlocker blocker8(ui->valueDoubleSpinBox); \
    const QSignalBlocker blocker9(ui->optionTypeComboBox); \
    const QSignalBlocker blocker10(ui->commentTextBox)

const int MAX_STR_LEN = 30000;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , logger("log.txt")
{
    ui->setupUi(this);
    customSetup();
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
    if(ui->optionsListWidget->currentRow() < 0) {
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
    updateCurItem();
}

inline void MainWindow::customSetup()
{
    const int64_t int64min = numeric_limits<int64_t>::min();
    const int64_t int64max = numeric_limits<int64_t>::max();
    ui->valueSpinBox->setMinimum(int64min);
    ui->valueSpinBox->setMaximum(int64max);
    ui->minSpinBox->setMinimum(int64min);
    ui->minSpinBox->setMaximum(int64max);
    ui->maxSpinBox->setMinimum(int64min);
    ui->maxSpinBox->setMaximum(int64max);
    connect(ui->valueSpinBox, SIGNAL(editingFinished()), this, SLOT(onValueSpinBoxEditingFinished()));
    connect(ui->minSpinBox, SIGNAL(editingFinished()), this, SLOT(onMinSpinBoxEditingFinished()));
    connect(ui->maxSpinBox, SIGNAL(editingFinished()), this, SLOT(onMaxSpinBoxEditingFinished()));
    connect(ui->valueDoubleSpinBox, SIGNAL(editingFinished()), this, SLOT(onValueDoubleSpinBoxEditingFinished()));
    connect(ui->minDoubleSpinBox, SIGNAL(editingFinished()), this, SLOT(onMinDoubleSpinBoxEditingFinished()));
    connect(ui->maxDoubleSpinBox, SIGNAL(editingFinished()), this, SLOT(onMaxDoubleSpinBoxEditingFinished()));
    connect(ui->valueTextEdit, SIGNAL(textChanged()), this, SLOT(onValueTextEditTextChanged()));
    connect(ui->valueCheckBox, SIGNAL(stateChanged(int)), this, SLOT(onValueCheckBoxStateChanged(int)));
    //connect(ui->spinBox, SIGNAL(editingFinished()), this, SLOT(onSpinBoxEditingFinished()));
    //connect(ui->pushButton, SIGNAL(clicked()), this, SLOT(onPushButtonClicked()));
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
    for (const auto& entry : fs::recursive_directory_iterator(projDir)) {
        const auto& path = entry.path().u8string();
        if (path.substr(path.find_last_of(".") + 1) == "cfg")
        {
            auto config = make_unique<Config>(path, logger);
            if(!config->parseConfig()) {
                continue;
            }
            if(config->parseError) {
                ui->statusbar->showMessage(tr("Ошибка при чтении ") +
                                           path.c_str() +
                                           ".");
            }
            configs.push_back(std::move(config));
        }
    }
    const int oldIndex = ui->tabsWidget->currentIndex();
    ui->tabsWidget->clear();
    for(uint64_t i = 0; i < configs.size(); ++i)
    {
        ui->tabsWidget->addTab(new QWidget, configs[i]->moduleName.c_str());
    }
    if(oldIndex < ui->tabsWidget->count() && oldIndex >= 0) {
        ui->tabsWidget->setCurrentIndex(oldIndex);
    }
}

inline void MainWindow::updateCurItem()
{
    if(currentOption().state == UNALTERED) {
        currentOption().state = ALTERED;
    }
    showThatConfigChanged();
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
    BLOCK_UNWANTED_SIGNALS();
    ui->optionTypeComboBox->setCurrentIndex(currentOption().type);
    ui->nameLineEdit->setText(currentOption().name.c_str());
    ui->commentTextBox->setPlainText(currentOption().comment.c_str());
    switch(currentOption().type)
    {
    case BOOL:
        ui->stackedWidget->setCurrentIndex(OptionType::BOOL);
        get<bool>(currentOption().value) ? ui->valueCheckBox->setCheckState(Qt::Checked) :
                                           ui->valueCheckBox->setCheckState(Qt::Unchecked);
        break;
    case INT:
        ui->stackedWidget->setCurrentIndex(OptionType::INT);
        ui->valueSpinBox->setValue(get<int64_t>(currentOption().value));
        if(holds_alternative<int64_t>(currentOption().min)) {
            ui->minSpinBox->setValue(get<int64_t>(currentOption().min));
        }
        else {
            ui->minSpinBox->setValue(numeric_limits<int64_t>::min());
        }
        if(holds_alternative<int64_t>(currentOption().max)) {
            ui->maxSpinBox->setValue(get<int64_t>(currentOption().max));
        }
        else {
            ui->maxSpinBox->setValue(numeric_limits<int64_t>::max());
        }
        break;
    case DOUBLE:
        ui->stackedWidget->setCurrentIndex(OptionType::DOUBLE);
        ui->valueDoubleSpinBox->setValue(get<double>(currentOption().value));
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
        ui->stackedWidget->setCurrentIndex(OptionType::STRING);
        ui->valueTextEdit->blockSignals(true);
        ui->valueTextEdit->setPlainText(get<string>(currentOption().value).c_str());
        ui->valueTextEdit->blockSignals(false);
        break;
    }
}

void MainWindow::saveConfig(const Config & config) const
{
    ofstream fout(config.filename);
    fout << "### " << config.moduleName << endl;
    int i = 0;
    for(const auto& option : config.options) {
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
        option->state = UNALTERED;
        ui->optionsListWidget->item(i++)->setText(option->toString().c_str());
    }
    ui->statusbar->showMessage(tr("Файл ") + config.filename.c_str() + " сохранен.");
    fout.close();
}

void MainWindow::showThatConfigChanged()
{
    ui->tabsWidget->setTabText(ui->tabsWidget->currentIndex(), tr("*") + currentConfig().moduleName.c_str());
}

void MainWindow::on_tabsWidget_currentChanged(int index)
{
    if(index < 0){
        return;
    }
    ui->optionsListWidget->clear();
    Config& config = *configs[static_cast<uint64_t>(index)];
    for(uint64_t i = 0; i < config.options.size(); ++i) {
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

void MainWindow::onValueSpinBoxEditingFinished()
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

void MainWindow::onMinSpinBoxEditingFinished()
{
    int currentRow = ui->optionsListWidget->currentRow();
    if(currentRow < 0)
    {
        return;
    }
    int64_t newMin = ui->minSpinBox->value();
    if(newMin > get<int64_t>(currentOption().value))
    {
        ui->statusbar->showMessage("Мин. значение не может превышать значения опции.");
        if(holds_alternative<int64_t>(currentOption().min)) {
            ui->minSpinBox->setValue(get<int64_t>(currentOption().min));
        }
        else {
            ui->minSpinBox->setValue(numeric_limits<int64_t>::min());
        }
        return;
    }
    currentOption().min = newMin;
}

void MainWindow::onMaxSpinBoxEditingFinished()
{
    int currentRow = ui->optionsListWidget->currentRow();
    if(currentRow < 0)
    {
        return;
    }
    int64_t newMax = ui->maxSpinBox->value();
    if(newMax < get<int64_t>(currentOption().value))
    {
        ui->statusbar->showMessage("Макс. значение не может быть меньше значения опции.");
        if(holds_alternative<int64_t>(currentOption().max)) {
            ui->maxSpinBox->setValue(get<int64_t>(currentOption().max));
        }
        else {
           ui->maxSpinBox->setValue(numeric_limits<int64_t>::max());
        }
        return;
    }
    currentOption().max = newMax;
}

void MainWindow::onValueDoubleSpinBoxEditingFinished()
{
    int currentRow = ui->optionsListWidget->currentRow();
    if(currentRow < 0)
    {
        return;
    }
    double newValue = ui->valueDoubleSpinBox->value();
    if(holds_alternative<double>(currentOption().min) &&  newValue < get<double>(currentOption().min))
    {
        ui->statusbar->showMessage("Значение типа double меньше указанного мин. значения.");
        ui->valueDoubleSpinBox->setValue(get<double>(currentOption().value));
        return;
    }
    if(holds_alternative<double>(currentOption().max) && newValue > get<double>(currentOption().max))
    {
        ui->statusbar->showMessage("Значение типа double превышает указанное макс. значение.");
        ui->valueDoubleSpinBox->setValue(get<double>(currentOption().value));
        return;
    }
    currentOption().value = newValue;
    updateCurItem();
}

void MainWindow::onMinDoubleSpinBoxEditingFinished()
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

void MainWindow::onMaxDoubleSpinBoxEditingFinished()
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

void MainWindow::onValueTextEditTextChanged()
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
    saveConfig(currentConfig());
}

void MainWindow::on_addButton_clicked()
{
    auto option = make_unique<Option>(NEW);
    option->type = BOOL;
    option->name = "sampleOption";
    option->value = true;
    option->comment = "comment";
    auto *newItem = new QListWidgetItem;
    newItem->setText(option->toString().c_str());
    const int curRow = ui->optionsListWidget->currentRow();
    ui->optionsListWidget->insertItem(curRow+1, newItem);
    currentConfig().options.insert(currentConfig().options.begin()+curRow+1, std::move(option));
    ui->optionsListWidget->setCurrentRow(curRow+1);
    showThatConfigChanged();
}

void MainWindow::on_removeButton_clicked()
{
    const int curRow = ui->optionsListWidget->currentRow();
    const int rowCount = ui->optionsListWidget->count();
    int nextRow = -1;
    currentConfig().options.erase(currentConfig().options.begin() + curRow);
    if(curRow == rowCount-1 && curRow != 0) {
        nextRow = curRow-1;
    }
    if(curRow != rowCount-1 && curRow == 0) {
        nextRow = 0;
    }
    if(curRow != rowCount-1 && curRow != 0) {
        nextRow = curRow;
    }
    ui->optionsListWidget->blockSignals(true);
    delete ui->optionsListWidget->takeItem(curRow);
    ui->optionsListWidget->blockSignals(false);
    if(nextRow < 0) {
        return;
    }
    ui->optionsListWidget->setCurrentRow(nextRow);
    updateInfo();
    showThatConfigChanged();
}

void MainWindow::on_updateButton_clicked()
{
    openProjDir(ui->projDirLineEdit->text().toUtf8().constData());
}

void MainWindow::on_helpButton_clicked()
{
    QMessageBox mb;
    mb.setText("Справка.");
    mb.exec();
}

void MainWindow::on_saveAllButton_clicked()
{
    for(const auto& config : configs) {
        saveConfig(*config);
    }
    ui->statusbar->showMessage("Все файлы сохранены.");
}

void MainWindow::onValueCheckBoxStateChanged(int state)
{
    if(ui->optionsListWidget->currentRow() < 0) {
        return;
    }
    currentOption().value = state == Qt::Checked;
    updateCurItem();
}
