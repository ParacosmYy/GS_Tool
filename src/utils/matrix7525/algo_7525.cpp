/**
 * @file algo_7525.cpp
 */
#include "matrix7525/algo_7525.h"
QVector<double> algo_7525::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
