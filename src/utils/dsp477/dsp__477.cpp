/**
 * @file dsp__477.cpp
 * @brief dsp__477 implementation
 */
#include "dsp477/dsp__477.h"
QVector<double> dsp__477::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

