/**
 * @file dsp__647.cpp
 * @brief dsp__647 implementation
 */
#include "dsp647/dsp__647.h"
QVector<double> dsp__647::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

