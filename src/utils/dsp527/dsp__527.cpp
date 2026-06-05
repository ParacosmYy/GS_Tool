/**
 * @file dsp__527.cpp
 * @brief dsp__527 implementation
 */
#include "dsp527/dsp__527.h"
QVector<double> dsp__527::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

