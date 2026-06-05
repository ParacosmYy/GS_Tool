/**
 * @file dsp__747.cpp
 * @brief dsp__747 implementation
 */
#include "dsp747/dsp__747.h"
QVector<double> dsp__747::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

