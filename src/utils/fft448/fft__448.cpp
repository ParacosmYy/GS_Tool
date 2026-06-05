/**
 * @file fft__448.cpp
 * @brief fft__448 implementation
 */
#include "fft448/fft__448.h"
QVector<double> fft__448::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

