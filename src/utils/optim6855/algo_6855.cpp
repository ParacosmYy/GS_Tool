/**
 * @file algo_6855.cpp
 */
#include "optim6855/algo_6855.h"
QVector<double> algo_6855::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
