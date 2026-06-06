/**
 * @file algo_7115.cpp
 */
#include "optim7115/algo_7115.h"
QVector<double> algo_7115::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
