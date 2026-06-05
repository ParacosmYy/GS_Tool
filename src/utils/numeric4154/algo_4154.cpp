/**
 * @file algo_4154.cpp
 */
#include "numeric4154/algo_4154.h"
QVector<double> algo_4154::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
