/**
 * @file fft__378.cpp
 * @brief fft__378 implementation
 */
#include "fft378/fft__378.h"
QVector<double> fft__378::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

