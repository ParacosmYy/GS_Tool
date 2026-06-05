/**
 * @file dsp__627.cpp
 * @brief dsp__627 implementation
 */
#include "dsp627/dsp__627.h"
QVector<double> dsp__627::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

