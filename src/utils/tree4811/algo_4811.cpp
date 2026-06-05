/**
 * @file algo_4811.cpp
 */
#include "tree4811/algo_4811.h"
QVector<double> algo_4811::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
