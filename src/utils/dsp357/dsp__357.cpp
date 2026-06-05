/**
 * @file dsp__357.cpp
 * @brief dsp__357 implementation
 */
#include "dsp357/dsp__357.h"
QVector<double> dsp__357::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

