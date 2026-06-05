/**
 * @file algo_3915.cpp
 */
#include "optim3915/algo_3915.h"
QVector<double> algo_3915::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
