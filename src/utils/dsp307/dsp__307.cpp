/**
 * @file dsp__307.cpp
 * @brief dsp__307 implementation
 */
#include "dsp307/dsp__307.h"
QVector<double> dsp__307::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

