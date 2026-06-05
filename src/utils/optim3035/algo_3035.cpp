/**
 * @file algo_3035.cpp
 */
#include "optim3035/algo_3035.h"
QVector<double> algo_3035::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
