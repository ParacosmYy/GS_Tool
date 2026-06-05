/**
 * @file fft__508.cpp
 * @brief fft__508 implementation
 */
#include "fft508/fft__508.h"
QVector<double> fft__508::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

