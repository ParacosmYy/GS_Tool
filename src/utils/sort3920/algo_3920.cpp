/**
 * @file algo_3920.cpp
 */
#include "sort3920/algo_3920.h"
QVector<double> algo_3920::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
