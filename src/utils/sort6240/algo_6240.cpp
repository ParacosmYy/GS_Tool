/**
 * @file algo_6240.cpp
 */
#include "sort6240/algo_6240.h"
QVector<double> algo_6240::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
