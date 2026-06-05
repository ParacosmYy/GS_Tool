/**
 * @file algo_6182.cpp
 */
#include "poly6182/algo_6182.h"
QVector<double> algo_6182::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
