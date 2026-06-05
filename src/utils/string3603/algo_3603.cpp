/**
 * @file algo_3603.cpp
 */
#include "string3603/algo_3603.h"
QVector<double> algo_3603::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
