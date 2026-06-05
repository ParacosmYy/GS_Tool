/**
 * @file algo_3368.cpp
 */
#include "fft3368/algo_3368.h"
QVector<double> algo_3368::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
