/**
 * @file algo_3651.cpp
 */
#include "tree3651/algo_3651.h"
QVector<double> algo_3651::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
