/**
 * @file dsp__447.cpp
 * @brief dsp__447 implementation
 */
#include "dsp447/dsp__447.h"
QVector<double> dsp__447::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

