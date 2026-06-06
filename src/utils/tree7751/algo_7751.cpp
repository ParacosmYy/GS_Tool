/**
 * @file algo_7751.cpp
 */
#include "tree7751/algo_7751.h"
QVector<double> algo_7751::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
