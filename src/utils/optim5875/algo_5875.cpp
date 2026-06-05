/**
 * @file algo_5875.cpp
 */
#include "optim5875/algo_5875.h"
QVector<double> algo_5875::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
