/**
 * @file algo_7014.cpp
 */
#include "numeric7014/algo_7014.h"
QVector<double> algo_7014::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
