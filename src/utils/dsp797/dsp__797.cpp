/**
 * @file dsp__797.cpp
 * @brief dsp__797 implementation
 */
#include "dsp797/dsp__797.h"
QVector<double> dsp__797::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

