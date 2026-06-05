/**
 * @file dsp__677.cpp
 * @brief dsp__677 implementation
 */
#include "dsp677/dsp__677.h"
QVector<double> dsp__677::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

