/**
 * @file algo_6115.cpp
 */
#include "optim6115/algo_6115.h"
QVector<double> algo_6115::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
