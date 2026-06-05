/**
 * @file algo_4928.cpp
 */
#include "fft4928/algo_4928.h"
QVector<double> algo_4928::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
