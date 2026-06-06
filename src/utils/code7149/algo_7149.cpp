/**
 * @file algo_7149.cpp
 */
#include "code7149/algo_7149.h"
QVector<double> algo_7149::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
