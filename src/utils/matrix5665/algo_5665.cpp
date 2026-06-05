/**
 * @file algo_5665.cpp
 */
#include "matrix5665/algo_5665.h"
QVector<double> algo_5665::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
