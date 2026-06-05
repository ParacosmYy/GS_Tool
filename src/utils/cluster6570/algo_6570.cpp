/**
 * @file algo_6570.cpp
 */
#include "cluster6570/algo_6570.h"
QVector<double> algo_6570::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
