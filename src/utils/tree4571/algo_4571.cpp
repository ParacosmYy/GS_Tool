/**
 * @file algo_4571.cpp
 */
#include "tree4571/algo_4571.h"
QVector<double> algo_4571::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
