/**
 * @file dsp__547.cpp
 * @brief dsp__547 implementation
 */
#include "dsp547/dsp__547.h"
QVector<double> dsp__547::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

