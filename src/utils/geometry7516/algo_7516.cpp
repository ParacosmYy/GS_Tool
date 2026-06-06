/**
 * @file algo_7516.cpp
 */
#include "geometry7516/algo_7516.h"
QVector<double> algo_7516::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
