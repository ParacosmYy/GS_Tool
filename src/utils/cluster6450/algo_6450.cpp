/**
 * @file algo_6450.cpp
 */
#include "cluster6450/algo_6450.h"
QVector<double> algo_6450::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
