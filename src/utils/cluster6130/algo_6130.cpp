/**
 * @file algo_6130.cpp
 */
#include "cluster6130/algo_6130.h"
QVector<double> algo_6130::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
