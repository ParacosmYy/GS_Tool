/**
 * @file fft__408.cpp
 * @brief fft__408 implementation
 */
#include "fft408/fft__408.h"
QVector<double> fft__408::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

