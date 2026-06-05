/**
 * @file algo_3375.cpp
 */
#include "optim3375/algo_3375.h"
QVector<double> algo_3375::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
