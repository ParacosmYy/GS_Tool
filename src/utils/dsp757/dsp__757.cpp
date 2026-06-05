/**
 * @file dsp__757.cpp
 * @brief dsp__757 implementation
 */
#include "dsp757/dsp__757.h"
QVector<double> dsp__757::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

