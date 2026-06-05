/**
 * @file algo_2975.cpp
 */
#include "optim2975/algo_2975.h"
QVector<double> algo_2975::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
