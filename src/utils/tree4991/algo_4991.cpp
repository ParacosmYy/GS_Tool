/**
 * @file algo_4991.cpp
 */
#include "tree4991/algo_4991.h"
QVector<double> algo_4991::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
