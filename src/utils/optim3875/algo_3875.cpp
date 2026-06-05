/**
 * @file algo_3875.cpp
 */
#include "optim3875/algo_3875.h"
QVector<double> algo_3875::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
