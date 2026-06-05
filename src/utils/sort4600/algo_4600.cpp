/**
 * @file algo_4600.cpp
 */
#include "sort4600/algo_4600.h"
QVector<double> algo_4600::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
