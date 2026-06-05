/**
 * @file dsp__397.cpp
 * @brief dsp__397 implementation
 */
#include "dsp397/dsp__397.h"
QVector<double> dsp__397::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

