/**
 * @file algo_6220.cpp
 */
#include "sort6220/algo_6220.h"
QVector<double> algo_6220::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
