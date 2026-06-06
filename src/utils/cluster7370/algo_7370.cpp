/**
 * @file algo_7370.cpp
 */
#include "cluster7370/algo_7370.h"
QVector<double> algo_7370::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
