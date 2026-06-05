/**
 * @file algo_5718.cpp
 */
#include "neural5718/algo_5718.h"
QVector<double> algo_5718::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
