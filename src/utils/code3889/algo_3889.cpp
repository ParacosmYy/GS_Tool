/**
 * @file algo_3889.cpp
 */
#include "code3889/algo_3889.h"
QVector<double> algo_3889::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
