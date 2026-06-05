/**
 * @file algo_5490.cpp
 */
#include "cluster5490/algo_5490.h"
QVector<double> algo_5490::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
