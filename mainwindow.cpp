#include <QShortcut>
#include <QMessageBox>
#include <QFileDialog>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "config.h"
#include <QPushButton>
#include <QHBoxLayout>
#include <iostream>
#include <filesystem>
using namespace std;
namespace fs = filesystem;

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
const string APP_NAME = "CFGReader";

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , logger(nullptr)
{
    ui->setupUi(this);
    customSetup();
}

MainWindow::~MainWindow()
{
    ui->projDirLineEdit->blockSignals(true);
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
    if(currentOption().name == newName) {
        return;
    }
    if(!regex_match(newName, Regex::optionName))
    {
        ui->statusbar->showMessage("Некорректное имя опции.");
        ui->nameLineEdit->setText(currentOption().name.c_str());
        return;
    }
    currentOption().name = newName;
    updateCurItem();
    saveBackup(currentConfig());
}

void MainWindow::tabsWidgetSetFocus()
{
    ui->tabsWidget->setFocus();
}

void MainWindow::optionsListSetFocus()
{
    ui->optionsListWidget->setFocus();
}

void MainWindow::comboBoxSetFocus()
{
    ui->optionTypeComboBox->setFocus();
}

void MainWindow::projDirLineEditSetFocus()
{
    ui->projDirLineEdit->setFocus();
}

void MainWindow::commentSetFocus()
{
    ui->commentTextBox->setFocus();
}

void MainWindow::customSetup()
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
    connect(ui->projDirLineEdit, SIGNAL(editingFinished()), this, SLOT(onProjDirLineEditEditingFinished()));
    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_U), this, SLOT(on_updateButton_clicked()));
    new QShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_A), this, SLOT(on_addButton_clicked()));
    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_R), this, SLOT(on_removeButton_clicked()));
    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_S), this, SLOT(on_saveButton_clicked()));
    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_O), this, SLOT(on_openPushButton_clicked()));
    new QShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_H), this, SLOT(on_helpButton_clicked()));
    new QShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_S), this, SLOT(on_saveAllButton_clicked()));

    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_T), this, SLOT(tabsWidgetSetFocus()));
    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_L), this, SLOT(optionsListSetFocus()));
    new QShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_C), this, SLOT(comboBoxSetFocus()));
    new QShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_O), this, SLOT(projDirLineEditSetFocus()));
    new QShortcut(QKeySequence(Qt::ALT + Qt::SHIFT + Qt::Key_O), this, SLOT(commentSetFocus()));
}

Option &MainWindow::currentOption()
{
    return *configs[static_cast<uint64_t>(ui->tabsWidget->currentIndex())]->options[static_cast<uint64_t>(ui->optionsListWidget->currentRow())];
}

Config &MainWindow::currentConfig()
{
    return *configs[static_cast<uint64_t>(ui->tabsWidget->currentIndex())];
}

void MainWindow::openProjDir(const string &projDir, bool reopen)
{
    static string oldProjDir = "";
    if((projDir == "" || oldProjDir == projDir) && !reopen) {
        return;
    }
    if(!fs::exists(projDir)) {
        ui->statusbar->showMessage("Такой директории не существует.");
        return;
    }
    oldProjDir = projDir;
    deleteAllBackups();
    bool projOpenedSuccessfully = true;
    configs.clear();
    logger.reset(new Logger(oldProjDir + "/cfgreader.log"));
    try {
        for (const auto& entry : fs::recursive_directory_iterator(projDir)) {
            const string path = entry.path().u8string();
            string filename, extension, backupPath;
            extractFilename(path, filename, extension, backupPath);
            auto config = make_unique<Config>(path, *logger);
            bool parsed = false;
            if (extension == "cfg" && filename.substr(0, 2) != ".#") {
                if(fs::exists(backupPath)) {
                    QMessageBox::StandardButton resBtn = QMessageBox::question( this, APP_NAME.c_str(),
                                                                                    tr("Файл ") +
                                                                                    filename.c_str() +
                                                                                    " имеет резервную копию. Загрузить ее вместо оригинала?",
                                                                                    QMessageBox::Cancel | QMessageBox::No | QMessageBox::Yes,
                                                                                    QMessageBox::Yes);
                    if(resBtn == QMessageBox::Yes) {
                        parsed = config->parseConfig(backupPath);
                        config->isAltered = true;
                    }
                    if(resBtn == QMessageBox::No) {
                        parsed = config->parseConfig();
                        fs::remove(backupPath);
                    }
                    if(resBtn == QMessageBox::Cancel) {
                        continue;
                    }
                }
                else {
                    parsed = config->parseConfig();
                }
                if(config->parseError) {
                    projOpenedSuccessfully = false;
                    ui->statusbar->showMessage(tr("Ошибка при чтении ") +
                                               filename.c_str() +
                                               ".");
                }
                if(parsed) {
                    configs.push_back(std::move(config));
                }
            }
        }
    } catch (fs::filesystem_error& e) {
        ui->statusbar->showMessage(e.what());
        return;
    }
    const int oldIndex = ui->tabsWidget->currentIndex();
    ui->tabsWidget->clear();
    for(uint64_t i = 0; i < configs.size(); ++i)
    {
        Config& config = *configs[i];
        ui->tabsWidget->addTab(new QWidget, config.moduleName.c_str());
        if(config.isAltered) {
            showThatConfigAltered(i);
        }
    }
    if(oldIndex < ui->tabsWidget->count() && oldIndex >= 0) {
        ui->tabsWidget->setCurrentIndex(oldIndex);
    }
    if(projOpenedSuccessfully) {
        ui->statusbar->showMessage("Проект успешно открыт.");
    }
}

void MainWindow::updateCurItem()
{
    if(currentOption().state == UNALTERED) {
        currentOption().state = ALTERED;
    }
    showThatConfigAltered(ui->tabsWidget->currentIndex());
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

void MainWindow::saveBackup(const Config &config) const
{
    string filename, extension, backupPath;
    extractFilename(config.path, filename, extension, backupPath);
    ofstream fout(backupPath);
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
        ui->optionsListWidget->item(i++)->setText(option->toString().c_str());
    }
    fout.close();
}

void MainWindow::saveConfig(uint64_t configIndex)
{
    string filename, extension, backupPath;
    Config &config = *configs[configIndex];
    extractFilename(config.path, filename, extension, backupPath);
    if(!fs::exists(backupPath)) {
        return;
    }
    if(fs::exists(config.path)) {
        fs::remove(config.path);
    }
    fs::copy(backupPath, config.path);
    if(fs::exists(backupPath)) {
        fs::remove(backupPath);
    }
    for(uint64_t i = 0; i < config.options.size(); ++i) {
        config.options[i]->state = UNALTERED;
        if(ui->tabsWidget->currentIndex() == static_cast<int>(configIndex)) {
            ui->optionsListWidget->item(static_cast<int>(i))->setText(config.options[i]->toString().c_str());
        }
    }
    ui->statusbar->showMessage(tr("Файл ") + filename.c_str() + " сохранен.");
    showThatConfigAltered(configIndex, false);
}

void MainWindow::showThatConfigAltered(uint64_t configIndex, bool altered)
{
    if(altered) {
        ui->tabsWidget->setTabText(static_cast<int>(configIndex), tr("*") + configs[configIndex]->moduleName.c_str());
    }
    else {
        ui->tabsWidget->setTabText(static_cast<int>(configIndex), configs[configIndex]->moduleName.c_str());
    }
}

void MainWindow::showThatConfigAltered(int configIndex, bool altered)
{
    showThatConfigAltered(static_cast<uint64_t>(configIndex), altered);
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

void MainWindow::onProjDirLineEditEditingFinished()
{
    openProjDir(ui->projDirLineEdit->text().toUtf8().constData());
}

void MainWindow::on_openPushButton_clicked()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
                                                 "~",
                                                 QFileDialog::ShowDirsOnly
                                                 | QFileDialog::DontResolveSymlinks);
    if(dir == "") {
        return;
    }
    ui->projDirLineEdit->setText(dir);
    openProjDir(dir.toUtf8().constData());
}

void MainWindow::onValueSpinBoxEditingFinished()
{
    int currentRow = ui->optionsListWidget->currentRow();
    if(currentRow < 0) {
        return;
    }
    int64_t newValue = ui->valueSpinBox->value();
    if(newValue == get<int64_t>(currentOption().value)) {
        return;
    }
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
    saveBackup(currentConfig());
}

void MainWindow::onMinSpinBoxEditingFinished()
{
    int currentRow = ui->optionsListWidget->currentRow();
    if(currentRow < 0) {
        return;
    }
    int64_t newMin = ui->minSpinBox->value();
    if(holds_alternative<int64_t>(currentOption().min) &&
       newMin == get<int64_t>(currentOption().min)) {
        return;
    }
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
    updateCurItem();
    saveBackup(currentConfig());
}

void MainWindow::onMaxSpinBoxEditingFinished()
{
    int currentRow = ui->optionsListWidget->currentRow();
    if(currentRow < 0) {
        return;
    }
    int64_t newMax = ui->maxSpinBox->value();
    if(holds_alternative<int64_t>(currentOption().max) &&
       newMax == get<int64_t>(currentOption().max)) {
        return;
    }
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
    updateCurItem();
    saveBackup(currentConfig());
}

void MainWindow::onValueDoubleSpinBoxEditingFinished()
{
    int currentRow = ui->optionsListWidget->currentRow();
    if(currentRow < 0) {
        return;
    }
    double newValue = ui->valueDoubleSpinBox->value();
    if(newValue == get<double>(currentOption().value)) {
        return;
    }
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
    saveBackup(currentConfig());
}

void MainWindow::onMinDoubleSpinBoxEditingFinished()
{
    int currentRow = ui->optionsListWidget->currentRow();
    if(currentRow < 0) {
        return;
    }
    double newMin = ui->minDoubleSpinBox->value();
    if(holds_alternative<double>(currentOption().min) &&
       newMin == get<double>(currentOption().min)) {
        return;
    }
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
    updateCurItem();
    saveBackup(currentConfig());
}

void MainWindow::onMaxDoubleSpinBoxEditingFinished()
{
    int currentRow = ui->optionsListWidget->currentRow();
    if(currentRow < 0) {
        return;
    }
    double newMax = ui->maxDoubleSpinBox->value();
    if(holds_alternative<double>(currentOption().max) &&
       newMax == get<double>(currentOption().max)) {
        return;
    }
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
    updateCurItem();
    saveBackup(currentConfig());
}

void MainWindow::on_commentTextBox_textChanged()
{
    onTextChanged(ui->commentTextBox, currentOption().comment);
    updateCurItem();
    saveBackup(currentConfig());
}

void MainWindow::onValueTextEditTextChanged()
{
    string strValue;
    onTextChanged(ui->valueTextEdit, strValue);
    currentOption().value = strValue;
    updateCurItem();
    saveBackup(currentConfig());
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
            currentOption().value = 0l;
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
            currentOption().value = static_cast<double>(get<bool>(currentOption().value));
            break;
        case INT:
            currentOption().value = static_cast<double>(get<int64_t>(currentOption().value));
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
    showThatConfigAltered(ui->tabsWidget->currentIndex());
    saveBackup(currentConfig());
}

void MainWindow::on_saveButton_clicked()
{
    saveConfig(static_cast<uint64_t>(ui->tabsWidget->currentIndex()));
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
    showThatConfigAltered(ui->tabsWidget->currentIndex());
    saveBackup(currentConfig());
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
    ui->optionsListWidget->setCurrentRow(nextRow);
    if(nextRow >= 0) {
        updateInfo();
    }
    showThatConfigAltered(ui->tabsWidget->currentIndex());
    saveBackup(currentConfig());
}

void MainWindow::on_updateButton_clicked()
{
    openProjDir(ui->projDirLineEdit->text().toUtf8().constData(), true);
}

void MainWindow::on_helpButton_clicked()
{
    QMessageBox mb;
    mb.setText("Справка.");
    mb.exec();
}

void MainWindow::on_saveAllButton_clicked()
{
    for(uint64_t i = 0; i < configs.size(); ++i) {
        saveConfig(i);
    }
}

void MainWindow::onValueCheckBoxStateChanged(int state)
{
    if(ui->optionsListWidget->currentRow() < 0) {
        return;
    }
    currentOption().value = state == Qt::Checked;
    updateCurItem();
    saveBackup(currentConfig());
}

void MainWindow::closeEvent (QCloseEvent *event)
{
    deleteAllBackups();
    event->accept();
}

void MainWindow::deleteAllBackups()
{
    for(const auto& config : configs) {
        string filename, extension, backupPath;
        extractFilename(config->path, filename, extension, backupPath);
        if(fs::exists(backupPath)) {
            fs::remove(backupPath);
        }
    }
}

void MainWindow::extractFilename(const std::string& path, std::string& filename, std::string& extension, std::string& backupPath) const
{
    filename = path.substr(path.find_last_of("\\/")+1);
    extension = filename.substr(filename.find_last_of(".")+1);
    backupPath = path.substr(0, path.find_last_of("/\\")) + "/.#" + filename;
}
