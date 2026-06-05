/**
 * @file algo_3171.cpp
 */
#include "tree3171/algo_3171.h"
QVector<double> algo_3171::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
