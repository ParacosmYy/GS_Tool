/**
 * @file algo_5990.cpp
 */
#include "cluster5990/algo_5990.h"
QVector<double> algo_5990::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
