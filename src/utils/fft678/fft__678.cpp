/**
 * @file fft__678.cpp
 * @brief fft__678 implementation
 */
#include "fft678/fft__678.h"
QVector<double> fft__678::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

