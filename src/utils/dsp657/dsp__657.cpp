/**
 * @file dsp__657.cpp
 * @brief dsp__657 implementation
 */
#include "dsp657/dsp__657.h"
QVector<double> dsp__657::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

