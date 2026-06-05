/**
 * @file algo_5015.cpp
 */
#include "optim5015/algo_5015.h"
QVector<double> algo_5015::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
