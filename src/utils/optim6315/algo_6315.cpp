/**
 * @file algo_6315.cpp
 */
#include "optim6315/algo_6315.h"
QVector<double> algo_6315::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
