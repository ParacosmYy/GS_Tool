/**
 * @file algo_3350.cpp
 */
#include "cluster3350/algo_3350.h"
QVector<double> algo_3350::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
