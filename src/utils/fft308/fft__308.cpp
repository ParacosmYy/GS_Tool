/**
 * @file fft__308.cpp
 * @brief fft__308 implementation
 */
#include "fft308/fft__308.h"
QVector<double> fft__308::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

