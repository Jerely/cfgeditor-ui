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

    void on_projDirLineEdit_editingFinished();

    void on_openPushButton_clicked();

    void on_valueSpinBox_editingFinished();

    void on_minValueSpinBox_editingFinished();

    void on_maxValueSpinBox_editingFinished();

    void on_doubleSpinBox_editingFinished();

    void on_minDoubleSpinBox_editingFinished();

    void on_maxDoubleSpinBox_editingFinished();

    //void on_valueLineEdit_editingFinished();

    void on_commentTextBox_textChanged();

    void on_valueTextEdit_textChanged();

private:
    Ui::MainWindow *ui;
    Logger logger;
    std::vector<std::unique_ptr<Config>> configs;
    inline void customSetup();
    inline Option& currentOption();
    void openProjDir(const std::string&);
    inline void updateCurItem();
    void onTextChanged(const QPlainTextEdit*, std::string&);
};
#endif // MAINWINDOW_H
