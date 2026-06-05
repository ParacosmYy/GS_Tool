/**
 * @file dsp__327.cpp
 * @brief dsp__327 implementation
 */
#include "dsp327/dsp__327.h"
QVector<double> dsp__327::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

