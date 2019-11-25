#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidget>
#include <QPlainTextEdit>
#include "config.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:

    void on_optionsListWidget_currentRowChanged(int currentRow);

    void on_nameLineEdit_editingFinished();

    void on_tabsWidget_currentChanged(int index);

    void onProjDirLineEditEditingFinished();

    void on_openPushButton_clicked();

    void onValueSpinBoxEditingFinished();

    void onMinSpinBoxEditingFinished();

    void onMaxSpinBoxEditingFinished();

    void onValueDoubleSpinBoxEditingFinished();

    void onMinDoubleSpinBoxEditingFinished();

    void onMaxDoubleSpinBoxEditingFinished();

    void on_commentTextBox_textChanged();

    void onValueTextEditTextChanged();

    void on_optionTypeComboBox_currentIndexChanged(int index);

    void on_saveButton_clicked();

    void on_addButton_clicked();

    void on_removeButton_clicked();

    void on_updateButton_clicked();

    void on_helpButton_clicked();

    void on_saveAllButton_clicked();

    void onValueCheckBoxStateChanged(int state);

private:
    Ui::MainWindow *ui;
    Logger logger;
    std::vector<std::unique_ptr<Config>> configs;
    void customSetup();
    Option& currentOption();
    Config& currentConfig();
    void openProjDir(const std::string&);
    void updateCurItem();
    void onTextChanged(const QPlainTextEdit*, std::string&);
    void updateInfo();
    void saveBackup(const Config&) const;
    void saveConfig(Config&);
    void showThatConfigAltered(bool altered = true);
    void extractPath(const std::string&, std::string&, std::string&) const;
    void closeEvent (QCloseEvent *event);
    void deleteAllBackups(const std::string&);
    void deleteBackup(const std::string&);
};
#endif // MAINWINDOW_H
