/**
 * @file algo_2995.cpp
 */
#include "optim2995/algo_2995.h"
QVector<double> algo_2995::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
