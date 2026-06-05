/**
 * @file dsp__727.cpp
 * @brief dsp__727 implementation
 */
#include "dsp727/dsp__727.h"
QVector<double> dsp__727::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

