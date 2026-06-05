/**
 * @file algo_4870.cpp
 */
#include "cluster4870/algo_4870.h"
QVector<double> algo_4870::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
