/**
 * @file algo_4500.cpp
 */
#include "sort4500/algo_4500.h"
QVector<double> algo_4500::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
