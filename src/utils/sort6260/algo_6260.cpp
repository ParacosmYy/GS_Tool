/**
 * @file algo_6260.cpp
 */
#include "sort6260/algo_6260.h"
QVector<double> algo_6260::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
