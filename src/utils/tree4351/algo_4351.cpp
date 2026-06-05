/**
 * @file algo_4351.cpp
 */
#include "tree4351/algo_4351.h"
QVector<double> algo_4351::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
