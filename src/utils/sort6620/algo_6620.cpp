/**
 * @file algo_6620.cpp
 */
#include "sort6620/algo_6620.h"
QVector<double> algo_6620::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
