/**
 * @file algo_7291.cpp
 */
#include "tree7291/algo_7291.h"
QVector<double> algo_7291::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
