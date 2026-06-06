/**
 * @file algo_7351.cpp
 */
#include "tree7351/algo_7351.h"
QVector<double> algo_7351::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
