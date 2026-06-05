/**
 * @file algo_6458.cpp
 */
#include "neural6458/algo_6458.h"
QVector<double> algo_6458::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
