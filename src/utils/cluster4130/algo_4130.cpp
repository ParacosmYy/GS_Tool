/**
 * @file algo_4130.cpp
 */
#include "cluster4130/algo_4130.h"
QVector<double> algo_4130::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
