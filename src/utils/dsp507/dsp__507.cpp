/**
 * @file dsp__507.cpp
 * @brief dsp__507 implementation
 */
#include "dsp507/dsp__507.h"
QVector<double> dsp__507::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

