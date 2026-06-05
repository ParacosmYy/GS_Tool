/**
 * @file dsp__707.cpp
 * @brief dsp__707 implementation
 */
#include "dsp707/dsp__707.h"
QVector<double> dsp__707::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

