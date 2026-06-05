/**
 * @file algo_6671.cpp
 */
#include "tree6671/algo_6671.h"
QVector<double> algo_6671::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
