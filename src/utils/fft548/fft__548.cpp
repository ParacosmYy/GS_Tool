/**
 * @file fft__548.cpp
 * @brief fft__548 implementation
 */
#include "fft548/fft__548.h"
QVector<double> fft__548::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

