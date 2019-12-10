#ifndef QTLONGLONGSPINBOX_H
#define QTLONGLONGSPINBOX_H
#include <QtWidgets/QAbstractSpinBox>

class QtLongLongSpinBox : public QAbstractSpinBox
{
    Q_OBJECT
public:
    explicit QtLongLongSpinBox(QWidget *parent = nullptr);

    qlonglong value() const;

    QString cleanText() const;

    qlonglong singleStep() const;
    void setSingleStep(qlonglong val);

    qlonglong minimum() const;
    void setMinimum(qlonglong min);

    qlonglong maximum() const;
    void setMaximum(qlonglong max);

    void setRange(qlonglong min, qlonglong max);

public slots:
    void setValue(qlonglong value);

signals:
    void valueChanged(qlonglong i);
    void valueChanged(const QString &text);

protected:
    virtual void keyPressEvent(QKeyEvent *event);
    virtual void focusOutEvent(QFocusEvent *event);
    virtual void stepBy(int steps);
    virtual StepEnabled stepEnabled() const;
    virtual QValidator::State validate(QString & input, int &pos) const;

private:
    void lineEditEditingFinalize();
    void selectCleanText();

private:
    qlonglong m_singleStep;
    qlonglong m_minimum;
    qlonglong m_maximum;
    qlonglong m_value;

private:
    Q_DISABLE_COPY(QtLongLongSpinBox)
};
#endif // QTLONGLONGSPINBOX_H
