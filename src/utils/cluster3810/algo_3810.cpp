/**
 * @file algo_3810.cpp
 */
#include "cluster3810/algo_3810.h"
QVector<double> algo_3810::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
