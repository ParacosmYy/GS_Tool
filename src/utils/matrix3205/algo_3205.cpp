/**
 * @file algo_3205.cpp
 */
#include "matrix3205/algo_3205.h"
QVector<double> algo_3205::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
