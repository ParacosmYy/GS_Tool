/**
 * @file algo_6875.cpp
 */
#include "optim6875/algo_6875.h"
QVector<double> algo_6875::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
