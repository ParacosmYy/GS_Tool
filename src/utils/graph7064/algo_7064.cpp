/**
 * @file algo_7064.cpp
 */
#include "graph7064/algo_7064.h"
QVector<double> algo_7064::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
