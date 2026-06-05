/**
 * @file algo_6515.cpp
 */
#include "optim6515/algo_6515.h"
QVector<double> algo_6515::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
