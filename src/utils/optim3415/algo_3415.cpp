/**
 * @file algo_3415.cpp
 */
#include "optim3415/algo_3415.h"
QVector<double> algo_3415::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
