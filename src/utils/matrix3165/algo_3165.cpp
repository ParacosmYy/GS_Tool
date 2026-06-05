/**
 * @file algo_3165.cpp
 */
#include "matrix3165/algo_3165.h"
QVector<double> algo_3165::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
