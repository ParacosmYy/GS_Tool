/**
 * @file algo_3105.cpp
 */
#include "matrix3105/algo_3105.h"
QVector<double> algo_3105::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
