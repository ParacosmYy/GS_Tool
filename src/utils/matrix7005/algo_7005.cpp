/**
 * @file algo_7005.cpp
 */
#include "matrix7005/algo_7005.h"
QVector<double> algo_7005::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
