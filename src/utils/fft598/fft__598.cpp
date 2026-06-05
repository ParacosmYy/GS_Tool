/**
 * @file fft__598.cpp
 * @brief fft__598 implementation
 */
#include "fft598/fft__598.h"
QVector<double> fft__598::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

