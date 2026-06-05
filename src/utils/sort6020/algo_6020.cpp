/**
 * @file algo_6020.cpp
 */
#include "sort6020/algo_6020.h"
QVector<double> algo_6020::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
