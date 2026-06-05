/**
 * @file algo_3570.cpp
 */
#include "cluster3570/algo_3570.h"
QVector<double> algo_3570::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
