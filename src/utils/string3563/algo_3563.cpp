/**
 * @file algo_3563.cpp
 */
#include "string3563/algo_3563.h"
QVector<double> algo_3563::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
