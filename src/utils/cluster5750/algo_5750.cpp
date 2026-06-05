/**
 * @file algo_5750.cpp
 */
#include "cluster5750/algo_5750.h"
QVector<double> algo_5750::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
