/**
 * @file algo_4594.cpp
 */
#include "numeric4594/algo_4594.h"
QVector<double> algo_4594::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
