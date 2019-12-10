#ifndef DOUBLESPINBOX_H
#define DOUBLESPINBOX_H
#include <QtWidgets/QAbstractSpinBox>

class DoubleSpinBox : public QAbstractSpinBox
{
    Q_OBJECT
public:
    explicit DoubleSpinBox(QWidget *parent = nullptr);

    double value() const;

    QString cleanText() const;

    double singleStep() const;
    void setSingleStep(double val);

    double minimum() const;
    void setMinimum(double min);

    double maximum() const;
    void setMaximum(double max);

    void setRange(double min, double max);

public slots:
    void setValue(double value);

signals:
    void valueChanged(double i);
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
    double m_singleStep;
    double m_minimum;
    double m_maximum;
    double m_value;

private:
    Q_DISABLE_COPY(DoubleSpinBox)
};
#endif // DOUBLESPINBOX_H
