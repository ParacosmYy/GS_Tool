/**
 * @file fft__478.cpp
 * @brief fft__478 implementation
 */
#include "fft478/fft__478.h"
QVector<double> fft__478::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

