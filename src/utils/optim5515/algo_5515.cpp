/**
 * @file algo_5515.cpp
 */
#include "optim5515/algo_5515.h"
QVector<double> algo_5515::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
