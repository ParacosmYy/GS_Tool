/**
 * @file algo_4371.cpp
 */
#include "tree4371/algo_4371.h"
QVector<double> algo_4371::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
