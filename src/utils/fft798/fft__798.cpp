/**
 * @file fft__798.cpp
 * @brief fft__798 implementation
 */
#include "fft798/fft__798.h"
QVector<double> fft__798::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

