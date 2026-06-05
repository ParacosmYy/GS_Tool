/**
 * @file algo_5165.cpp
 */
#include "matrix5165/algo_5165.h"
QVector<double> algo_5165::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
