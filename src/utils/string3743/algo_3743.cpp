/**
 * @file algo_3743.cpp
 */
#include "string3743/algo_3743.h"
QVector<double> algo_3743::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
