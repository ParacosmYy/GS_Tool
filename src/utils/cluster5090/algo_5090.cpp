/**
 * @file algo_5090.cpp
 */
#include "cluster5090/algo_5090.h"
QVector<double> algo_5090::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
