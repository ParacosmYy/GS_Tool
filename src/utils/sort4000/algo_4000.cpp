/**
 * @file algo_4000.cpp
 */
#include "sort4000/algo_4000.h"
QVector<double> algo_4000::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
