/**
 * @file algo_4830.cpp
 */
#include "cluster4830/algo_4830.h"
QVector<double> algo_4830::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
