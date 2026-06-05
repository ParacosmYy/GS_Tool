/**
 * @file fft__528.cpp
 * @brief fft__528 implementation
 */
#include "fft528/fft__528.h"
QVector<double> fft__528::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

