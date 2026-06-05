/**
 * @file algo_3160.cpp
 */
#include "sort3160/algo_3160.h"
QVector<double> algo_3160::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
