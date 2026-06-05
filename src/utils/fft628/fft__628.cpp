/**
 * @file fft__628.cpp
 * @brief fft__628 implementation
 */
#include "fft628/fft__628.h"
QVector<double> fft__628::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

