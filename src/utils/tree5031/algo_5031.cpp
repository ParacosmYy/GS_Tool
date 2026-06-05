/**
 * @file algo_5031.cpp
 */
#include "tree5031/algo_5031.h"
QVector<double> algo_5031::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
