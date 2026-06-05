/**
 * @file algo_4370.cpp
 */
#include "cluster4370/algo_4370.h"
QVector<double> algo_4370::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
