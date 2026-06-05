/**
 * @file algo_6325.cpp
 */
#include "matrix6325/algo_6325.h"
QVector<double> algo_6325::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
