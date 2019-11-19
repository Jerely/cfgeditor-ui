#include "qlonglongspinbox.h"

QLongLongSpinBox::QLongLongSpinBox(QWidget *)
{
    connect(lineEdit(), SIGNAL(textEdited(QString)), this, SLOT(onEditFinished()));
}

QLongLongSpinBox::~QLongLongSpinBox() {}

qlonglong QLongLongSpinBox::value() const
{
    return m_value;
}

qlonglong QLongLongSpinBox::minimum() const
{
    return m_minimum;
}

void QLongLongSpinBox::setMinimum(qlonglong min)
{
    m_minimum = min;
}

qlonglong QLongLongSpinBox::maximum() const
{
    return m_maximum;
}

void QLongLongSpinBox::setMaximum(qlonglong max)
{
    m_maximum = max;
}

void QLongLongSpinBox::setRange(qlonglong min, qlonglong max)
{
    setMinimum(min);
    setMaximum(max);
}

void QLongLongSpinBox::stepBy(int steps)
{
    auto new_value = m_value;
    if (steps < 0 && new_value + steps > new_value) {
        new_value = std::numeric_limits<qlonglong>::min();
    }
    else if (steps > 0 && new_value + steps < new_value) {
        new_value = std::numeric_limits<qlonglong>::max();
    }
    else {
        new_value += steps;
    }

    lineEdit()->setText(textFromValue(new_value));
    setValue(new_value);
}

QValidator::State QLongLongSpinBox::validate(QString &input, int &) const
{
    bool ok;
    qlonglong val = input.toLongLong(&ok);
    if (!ok)
        return QValidator::Invalid;

    if (val < m_minimum || val > m_maximum)
        return QValidator::Invalid;

    return QValidator::Acceptable;
}

qlonglong QLongLongSpinBox::valueFromText(const QString &text) const
{
    return text.toLongLong();
}

QString QLongLongSpinBox::textFromValue(qlonglong val) const
{
    return QString::number(val);
}

QAbstractSpinBox::StepEnabled QLongLongSpinBox::stepEnabled() const
{
    return StepUpEnabled | StepDownEnabled;
}

void QLongLongSpinBox::setValue(qlonglong val)
{
    if (m_value != val) {
        lineEdit()->setText(textFromValue(val));
        m_value = val;
    }
}

void QLongLongSpinBox::onEditFinished()
{
    QString input = lineEdit()->text();
    int pos = 0;
    if (QValidator::Acceptable == validate(input, pos))
        setValue(valueFromText(input));
    else
        lineEdit()->setText(textFromValue(m_value));
}
