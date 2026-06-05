/**
 * @file algo_3808.cpp
 */
#include "fft3808/algo_3808.h"
QVector<double> algo_3808::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
