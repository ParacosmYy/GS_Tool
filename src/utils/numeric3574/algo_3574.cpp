/**
 * @file algo_3574.cpp
 */
#include "numeric3574/algo_3574.h"
QVector<double> algo_3574::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
