/**
 * @file algo_3871.cpp
 */
#include "tree3871/algo_3871.h"
QVector<double> algo_3871::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
