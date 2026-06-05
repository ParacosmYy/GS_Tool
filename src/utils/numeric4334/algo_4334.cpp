/**
 * @file algo_4334.cpp
 */
#include "numeric4334/algo_4334.h"
QVector<double> algo_4334::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
