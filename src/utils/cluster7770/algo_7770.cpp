/**
 * @file algo_7770.cpp
 */
#include "cluster7770/algo_7770.h"
QVector<double> algo_7770::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
