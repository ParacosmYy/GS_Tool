/**
 * @file algo_5788.cpp
 */
#include "fft5788/algo_5788.h"
QVector<double> algo_5788::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
