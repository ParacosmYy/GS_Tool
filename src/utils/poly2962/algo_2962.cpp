/**
 * @file algo_2962.cpp
 */
#include "poly2962/algo_2962.h"
QVector<double> algo_2962::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
