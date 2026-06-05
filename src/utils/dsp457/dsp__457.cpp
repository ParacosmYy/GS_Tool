/**
 * @file dsp__457.cpp
 * @brief dsp__457 implementation
 */
#include "dsp457/dsp__457.h"
QVector<double> dsp__457::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

