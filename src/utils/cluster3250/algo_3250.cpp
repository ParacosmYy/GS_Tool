/**
 * @file algo_3250.cpp
 */
#include "cluster3250/algo_3250.h"
QVector<double> algo_3250::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
