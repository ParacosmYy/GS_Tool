/**
 * @file fft__498.cpp
 * @brief fft__498 implementation
 */
#include "fft498/fft__498.h"
QVector<double> fft__498::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

