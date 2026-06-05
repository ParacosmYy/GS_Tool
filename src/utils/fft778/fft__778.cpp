/**
 * @file fft__778.cpp
 * @brief fft__778 implementation
 */
#include "fft778/fft__778.h"
QVector<double> fft__778::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

