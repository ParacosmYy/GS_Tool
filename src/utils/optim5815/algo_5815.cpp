/**
 * @file algo_5815.cpp
 */
#include "optim5815/algo_5815.h"
QVector<double> algo_5815::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
