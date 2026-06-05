/**
 * @file algo_3334.cpp
 */
#include "numeric3334/algo_3334.h"
QVector<double> algo_3334::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
