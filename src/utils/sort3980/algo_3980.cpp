/**
 * @file algo_3980.cpp
 */
#include "sort3980/algo_3980.h"
QVector<double> algo_3980::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
