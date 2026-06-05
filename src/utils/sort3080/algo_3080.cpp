/**
 * @file algo_3080.cpp
 */
#include "sort3080/algo_3080.h"
QVector<double> algo_3080::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
