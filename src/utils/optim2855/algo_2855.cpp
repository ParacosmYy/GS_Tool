/**
 * @file algo_2855.cpp
 */
#include "optim2855/algo_2855.h"
QVector<double> algo_2855::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
