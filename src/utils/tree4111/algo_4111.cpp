/**
 * @file algo_4111.cpp
 */
#include "tree4111/algo_4111.h"
QVector<double> algo_4111::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
