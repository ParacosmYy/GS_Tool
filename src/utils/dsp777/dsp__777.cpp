/**
 * @file dsp__777.cpp
 * @brief dsp__777 implementation
 */
#include "dsp777/dsp__777.h"
QVector<double> dsp__777::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

