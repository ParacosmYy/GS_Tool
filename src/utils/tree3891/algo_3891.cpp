/**
 * @file algo_3891.cpp
 */
#include "tree3891/algo_3891.h"
QVector<double> algo_3891::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
