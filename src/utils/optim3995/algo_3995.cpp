/**
 * @file algo_3995.cpp
 */
#include "optim3995/algo_3995.h"
QVector<double> algo_3995::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
