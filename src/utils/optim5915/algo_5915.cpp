/**
 * @file algo_5915.cpp
 */
#include "optim5915/algo_5915.h"
QVector<double> algo_5915::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
