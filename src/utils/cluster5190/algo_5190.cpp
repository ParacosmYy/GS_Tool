/**
 * @file algo_5190.cpp
 */
#include "cluster5190/algo_5190.h"
QVector<double> algo_5190::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
