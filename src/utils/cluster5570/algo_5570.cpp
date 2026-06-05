/**
 * @file algo_5570.cpp
 */
#include "cluster5570/algo_5570.h"
QVector<double> algo_5570::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
