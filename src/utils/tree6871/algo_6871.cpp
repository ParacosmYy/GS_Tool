/**
 * @file algo_6871.cpp
 */
#include "tree6871/algo_6871.h"
QVector<double> algo_6871::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
