/**
 * @file dsp__407.cpp
 * @brief dsp__407 implementation
 */
#include "dsp407/dsp__407.h"
QVector<double> dsp__407::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

