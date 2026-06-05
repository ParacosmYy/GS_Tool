/**
 * @file fft__428.cpp
 * @brief fft__428 implementation
 */
#include "fft428/fft__428.h"
QVector<double> fft__428::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

