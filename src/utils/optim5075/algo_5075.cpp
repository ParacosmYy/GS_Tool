/**
 * @file algo_5075.cpp
 */
#include "optim5075/algo_5075.h"
QVector<double> algo_5075::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
