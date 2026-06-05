/**
 * @file dsp__607.cpp
 * @brief dsp__607 implementation
 */
#include "dsp607/dsp__607.h"
QVector<double> dsp__607::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

