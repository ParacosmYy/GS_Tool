/**
 * @file algo_5983.cpp
 */
#include "string5983/algo_5983.h"
QVector<double> algo_5983::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
