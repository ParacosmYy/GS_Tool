/**
 * @file algo_6225.cpp
 */
#include "matrix6225/algo_6225.h"
QVector<double> algo_6225::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
