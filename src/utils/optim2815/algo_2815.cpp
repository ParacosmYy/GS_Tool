/**
 * @file algo_2815.cpp
 */
#include "optim2815/algo_2815.h"
QVector<double> algo_2815::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
