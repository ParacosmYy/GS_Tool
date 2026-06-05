/**
 * @file algo_3429.cpp
 */
#include "code3429/algo_3429.h"
QVector<double> algo_3429::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
