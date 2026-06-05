/**
 * @file algo_3729.cpp
 */
#include "code3729/algo_3729.h"
QVector<double> algo_3729::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
