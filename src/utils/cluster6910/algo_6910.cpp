/**
 * @file algo_6910.cpp
 */
#include "cluster6910/algo_6910.h"
QVector<double> algo_6910::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
