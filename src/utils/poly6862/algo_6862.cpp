/**
 * @file algo_6862.cpp
 */
#include "poly6862/algo_6862.h"
QVector<double> algo_6862::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
