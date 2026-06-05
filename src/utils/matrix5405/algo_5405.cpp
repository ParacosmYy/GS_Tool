/**
 * @file algo_5405.cpp
 */
#include "matrix5405/algo_5405.h"
QVector<double> algo_5405::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
