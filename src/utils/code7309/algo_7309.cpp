/**
 * @file algo_7309.cpp
 */
#include "code7309/algo_7309.h"
QVector<double> algo_7309::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
