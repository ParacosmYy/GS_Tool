/**
 * @file fft__698.cpp
 * @brief fft__698 implementation
 */
#include "fft698/fft__698.h"
QVector<double> fft__698::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

