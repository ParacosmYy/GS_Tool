/**
 * @file algo_5023.cpp
 */
#include "string5023/algo_5023.h"
QVector<double> algo_5023::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
