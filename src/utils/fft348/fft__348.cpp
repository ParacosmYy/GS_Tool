/**
 * @file fft__348.cpp
 * @brief fft__348 implementation
 */
#include "fft348/fft__348.h"
QVector<double> fft__348::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

