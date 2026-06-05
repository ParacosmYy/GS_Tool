/**
 * @file algo_3070.cpp
 */
#include "cluster3070/algo_3070.h"
QVector<double> algo_3070::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
