/**
 * @file algo_5985.cpp
 */
#include "matrix5985/algo_5985.h"
QVector<double> algo_5985::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
