/**
 * @file algo_3151.cpp
 */
#include "tree3151/algo_3151.h"
QVector<double> algo_3151::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
