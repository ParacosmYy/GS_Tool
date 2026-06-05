/**
 * @file algo_3845.cpp
 */
#include "matrix3845/algo_3845.h"
QVector<double> algo_3845::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
