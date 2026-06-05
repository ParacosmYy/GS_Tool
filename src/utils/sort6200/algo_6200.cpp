/**
 * @file algo_6200.cpp
 */
#include "sort6200/algo_6200.h"
QVector<double> algo_6200::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
