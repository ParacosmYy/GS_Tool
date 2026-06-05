/**
 * @file algo_5478.cpp
 */
#include "neural5478/algo_5478.h"
QVector<double> algo_5478::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
