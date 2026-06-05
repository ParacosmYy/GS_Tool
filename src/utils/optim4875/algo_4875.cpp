/**
 * @file algo_4875.cpp
 */
#include "optim4875/algo_4875.h"
QVector<double> algo_4875::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
