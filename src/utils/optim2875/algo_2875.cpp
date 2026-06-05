/**
 * @file algo_2875.cpp
 */
#include "optim2875/algo_2875.h"
QVector<double> algo_2875::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
