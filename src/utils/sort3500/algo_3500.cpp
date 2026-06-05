/**
 * @file algo_3500.cpp
 */
#include "sort3500/algo_3500.h"
QVector<double> algo_3500::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
