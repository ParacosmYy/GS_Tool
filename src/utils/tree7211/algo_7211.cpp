/**
 * @file algo_7211.cpp
 */
#include "tree7211/algo_7211.h"
QVector<double> algo_7211::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
