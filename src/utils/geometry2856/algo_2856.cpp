/**
 * @file algo_2856.cpp
 */
#include "geometry2856/algo_2856.h"
QVector<double> algo_2856::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
