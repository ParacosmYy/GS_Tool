/**
 * @file fft__358.cpp
 * @brief fft__358 implementation
 */
#include "fft358/fft__358.h"
QVector<double> fft__358::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

