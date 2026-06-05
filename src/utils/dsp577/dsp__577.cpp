/**
 * @file dsp__577.cpp
 * @brief dsp__577 implementation
 */
#include "dsp577/dsp__577.h"
QVector<double> dsp__577::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

