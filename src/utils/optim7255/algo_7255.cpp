/**
 * @file algo_7255.cpp
 */
#include "optim7255/algo_7255.h"
QVector<double> algo_7255::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
