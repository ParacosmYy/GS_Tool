/**
 * @file fft__578.cpp
 * @brief fft__578 implementation
 */
#include "fft578/fft__578.h"
QVector<double> fft__578::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

