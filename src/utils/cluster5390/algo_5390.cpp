/**
 * @file algo_5390.cpp
 */
#include "cluster5390/algo_5390.h"
QVector<double> algo_5390::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
