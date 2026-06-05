/**
 * @file algo_3481.cpp
 */
#include "interp3481/algo_3481.h"
QVector<double> algo_3481::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
