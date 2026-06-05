/**
 * @file algo_4151.cpp
 */
#include "tree4151/algo_4151.h"
QVector<double> algo_4151::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
