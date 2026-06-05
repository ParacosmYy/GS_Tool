/**
 * @file algo_4574.cpp
 */
#include "numeric4574/algo_4574.h"
QVector<double> algo_4574::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
