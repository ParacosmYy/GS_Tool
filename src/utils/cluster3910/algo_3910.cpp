/**
 * @file algo_3910.cpp
 */
#include "cluster3910/algo_3910.h"
QVector<double> algo_3910::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
