/**
 * @file algo_7515.cpp
 */
#include "optim7515/algo_7515.h"
QVector<double> algo_7515::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
