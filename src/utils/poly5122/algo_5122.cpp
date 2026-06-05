/**
 * @file algo_5122.cpp
 */
#include "poly5122/algo_5122.h"
QVector<double> algo_5122::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
