/**
 * @file algo_3460.cpp
 */
#include "sort3460/algo_3460.h"
QVector<double> algo_3460::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
