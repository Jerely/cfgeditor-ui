#include "defaultdoubleprecision.h"
#include "doublespinbox.h"
#include <QtWidgets/QLineEdit>
#include <QEvent>
#include <QKeyEvent>
#include <regex>
using namespace std;

static const regex doubleValue("^[+-]?((\\d+)|(\\d+\\.\\d*)|(\\d*\\.\\d+))([Ee][+-]?\\d*)?$");

static const int DOUBLE_MAX_LEN = 300;

DoubleSpinBox::DoubleSpinBox(QWidget *parent) :
    QAbstractSpinBox(parent)
{
    m_minimum = std::numeric_limits<double>::lowest();
    m_maximum = std::numeric_limits<double>::max();
    m_value = 0;
    m_singleStep = 1;

    setValue(m_value);
}

double DoubleSpinBox::value() const
{
    return m_value;
}

void DoubleSpinBox::setValue(double expectedNewValue)
{
    const QString newValueString = QString::number(expectedNewValue, 'g', DEFAULT_DOUBLE_PRECISION);
    lineEdit()->setText(newValueString);
    if (m_value != expectedNewValue) {
        m_value = expectedNewValue;

        emit valueChanged(expectedNewValue);
        emit valueChanged(newValueString);
    }
}

QString DoubleSpinBox::cleanText() const
{
    return QString::number(m_value, 'g', DEFAULT_DOUBLE_PRECISION);
}

double DoubleSpinBox::singleStep() const
{
    return m_singleStep;
}

void DoubleSpinBox::setSingleStep(double step)
{
    m_singleStep = step;
}

double DoubleSpinBox::minimum() const
{
    return m_minimum;
}

void DoubleSpinBox::setMinimum(double min)
{
    m_minimum = min;
    if (m_maximum < m_minimum) {
        m_maximum = m_minimum;
    }

    setValue(m_value);
}

double DoubleSpinBox::maximum() const
{
    return m_maximum;
}

void DoubleSpinBox::setMaximum(double max)
{
    m_maximum = max;
    if (m_maximum < m_minimum) {
        m_maximum = m_minimum;
    }

    setValue(m_value);
}

void DoubleSpinBox::setRange(double min, double max)
{
    if (min < max) {
        m_minimum = min;
        m_maximum = max;
    }
    else {
        m_minimum = max;
        m_maximum = min;
    }

    setValue(m_value);
}

void DoubleSpinBox::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Enter:
    case Qt::Key_Return:
        selectCleanText();
        lineEditEditingFinalize();
    }

    QAbstractSpinBox::keyPressEvent(event);
}

void DoubleSpinBox::focusOutEvent(QFocusEvent *event)
{
    lineEditEditingFinalize();

    QAbstractSpinBox::focusOutEvent(event);
}

QAbstractSpinBox::StepEnabled DoubleSpinBox::stepEnabled() const
{
    if (isReadOnly()) {
        return nullptr;
    }

    StepEnabled se = nullptr;
    if (wrapping() || m_value < m_maximum) {
        se |= StepUpEnabled;
    }
    if (wrapping() || m_value > m_minimum) {
        se |= StepDownEnabled;
    }

    return se;
}

void DoubleSpinBox::stepBy(int steps)
{
    if (isReadOnly()) {
        return;
    }

    //if(!regex_match(lineEdit()->text().toUtf8().constData(), doubleWithoutExp)) {
    //    return;
    //}

    if (QString::number(m_value, 'g', DEFAULT_DOUBLE_PRECISION) != lineEdit()->text()) {
        lineEditEditingFinalize();
    }

    double newValue = m_value + (steps * m_singleStep);
    if (wrapping()) {
        // emulating the behavior of QSpinBox
        if (newValue > m_maximum) {
            if (m_value == m_maximum) {
                newValue = m_minimum;
            }
            else {
                newValue = m_maximum;
            }
        }
        else if (newValue < m_minimum) {
            if (m_value == m_minimum) {
                newValue = m_maximum;
            }
            else {
                newValue = m_minimum;
            }
        }
    }

    setValue(newValue);
    selectCleanText();
}

QValidator::State DoubleSpinBox::validate(QString &input, int &) const
{
    if(input.length() > DOUBLE_MAX_LEN) {
        return QValidator::Invalid;
    }
    bool ok;
    const double value = input.toDouble(&ok);
    if (input == "-"
        || input == "+"
        || input.isEmpty()
        || regex_match(input.toUtf8().constData(), doubleValue)
        || (ok
            && value >= m_minimum
            && value <= m_maximum))
    {
        return QValidator::Acceptable;
    }
    return QValidator::Invalid;
}

void DoubleSpinBox::lineEditEditingFinalize()
{
    const QString text = lineEdit()->text();
    bool ok;
    double value = text.toDouble(&ok);
    if (ok) {
        setValue(value);
        return;
    }
    setValue(m_value);
}

void DoubleSpinBox::selectCleanText()
{
    lineEdit()->setSelection(0, lineEdit()->text().length());
}
