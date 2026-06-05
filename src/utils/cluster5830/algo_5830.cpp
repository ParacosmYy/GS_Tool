/**
 * @file algo_5830.cpp
 */
#include "cluster5830/algo_5830.h"
QVector<double> algo_5830::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
