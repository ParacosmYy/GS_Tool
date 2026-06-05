/**
 * @file fft__558.cpp
 * @brief fft__558 implementation
 */
#include "fft558/fft__558.h"
QVector<double> fft__558::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

