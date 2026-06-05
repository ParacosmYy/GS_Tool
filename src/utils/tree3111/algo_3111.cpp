/**
 * @file algo_3111.cpp
 */
#include "tree3111/algo_3111.h"
QVector<double> algo_3111::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
