/**
 * @file dsp__347.cpp
 * @brief dsp__347 implementation
 */
#include "dsp347/dsp__347.h"
QVector<double> dsp__347::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

