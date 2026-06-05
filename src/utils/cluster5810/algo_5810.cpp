/**
 * @file algo_5810.cpp
 */
#include "cluster5810/algo_5810.h"
QVector<double> algo_5810::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
