/**
 * @file algo_7414.cpp
 */
#include "numeric7414/algo_7414.h"
QVector<double> algo_7414::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
