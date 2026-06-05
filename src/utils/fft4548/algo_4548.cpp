/**
 * @file algo_4548.cpp
 */
#include "fft4548/algo_4548.h"
QVector<double> algo_4548::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
