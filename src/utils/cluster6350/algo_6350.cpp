/**
 * @file algo_6350.cpp
 */
#include "cluster6350/algo_6350.h"
QVector<double> algo_6350::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
