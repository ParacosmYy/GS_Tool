/**
 * @file algo_6582.cpp
 */
#include "poly6582/algo_6582.h"
QVector<double> algo_6582::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
