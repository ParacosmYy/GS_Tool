/**
 * @file algo_4646.cpp
 */
#include "signal4646/algo_4646.h"
QVector<double> algo_4646::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
