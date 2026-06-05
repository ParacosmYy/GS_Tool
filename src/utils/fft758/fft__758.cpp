/**
 * @file fft__758.cpp
 * @brief fft__758 implementation
 */
#include "fft758/fft__758.h"
QVector<double> fft__758::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

