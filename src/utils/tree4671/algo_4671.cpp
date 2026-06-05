/**
 * @file algo_4671.cpp
 */
#include "tree4671/algo_4671.h"
QVector<double> algo_4671::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
