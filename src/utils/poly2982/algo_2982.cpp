/**
 * @file algo_2982.cpp
 */
#include "poly2982/algo_2982.h"
QVector<double> algo_2982::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
