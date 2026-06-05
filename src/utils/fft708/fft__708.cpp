/**
 * @file fft__708.cpp
 * @brief fft__708 implementation
 */
#include "fft708/fft__708.h"
QVector<double> fft__708::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

