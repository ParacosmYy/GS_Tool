/**
 * @file algo_4471.cpp
 */
#include "tree4471/algo_4471.h"
QVector<double> algo_4471::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
