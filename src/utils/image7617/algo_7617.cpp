/**
 * @file algo_7617.cpp
 */
#include "image7617/algo_7617.h"
QVector<double> algo_7617::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
