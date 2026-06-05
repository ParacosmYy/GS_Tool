/**
 * @file algo_3275.cpp
 */
#include "optim3275/algo_3275.h"
QVector<double> algo_3275::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
