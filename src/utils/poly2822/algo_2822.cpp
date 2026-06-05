/**
 * @file algo_2822.cpp
 */
#include "poly2822/algo_2822.h"
QVector<double> algo_2822::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
