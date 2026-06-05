/**
 * @file dsp__497.cpp
 * @brief dsp__497 implementation
 */
#include "dsp497/dsp__497.h"
QVector<double> dsp__497::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

