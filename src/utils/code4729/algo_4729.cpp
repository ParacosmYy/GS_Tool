/**
 * @file algo_4729.cpp
 */
#include "code4729/algo_4729.h"
QVector<double> algo_4729::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
