/**
 * @file algo_6230.cpp
 */
#include "cluster6230/algo_6230.h"
QVector<double> algo_6230::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
