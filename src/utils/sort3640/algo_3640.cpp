/**
 * @file algo_3640.cpp
 */
#include "sort3640/algo_3640.h"
QVector<double> algo_3640::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
