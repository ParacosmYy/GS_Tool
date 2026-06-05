/**
 * @file algo_3475.cpp
 */
#include "optim3475/algo_3475.h"
QVector<double> algo_3475::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
