/**
 * @file algo_2931.cpp
 */
#include "tree2931/algo_2931.h"
QVector<double> algo_2931::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
