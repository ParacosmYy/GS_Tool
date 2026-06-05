/**
 * @file algo_6311.cpp
 */
#include "tree6311/algo_6311.h"
QVector<double> algo_6311::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
