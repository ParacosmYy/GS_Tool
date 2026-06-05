/**
 * @file algo_2826.cpp
 */
#include "signal2826/algo_2826.h"
QVector<double> algo_2826::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
