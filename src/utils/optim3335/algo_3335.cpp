/**
 * @file algo_3335.cpp
 */
#include "optim3335/algo_3335.h"
QVector<double> algo_3335::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
