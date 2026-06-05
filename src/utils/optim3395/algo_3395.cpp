/**
 * @file algo_3395.cpp
 */
#include "optim3395/algo_3395.h"
QVector<double> algo_3395::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
