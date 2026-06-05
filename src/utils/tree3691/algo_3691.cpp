/**
 * @file algo_3691.cpp
 */
#include "tree3691/algo_3691.h"
QVector<double> algo_3691::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
