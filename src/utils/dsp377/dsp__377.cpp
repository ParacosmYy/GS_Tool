/**
 * @file dsp__377.cpp
 * @brief dsp__377 implementation
 */
#include "dsp377/dsp__377.h"
QVector<double> dsp__377::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

