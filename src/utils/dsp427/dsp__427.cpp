/**
 * @file dsp__427.cpp
 * @brief dsp__427 implementation
 */
#include "dsp427/dsp__427.h"
QVector<double> dsp__427::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

