/**
 * @file algo_3140.cpp
 */
#include "sort3140/algo_3140.h"
QVector<double> algo_3140::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
