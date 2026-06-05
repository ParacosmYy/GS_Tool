/**
 * @file algo_3403.cpp
 */
#include "string3403/algo_3403.h"
QVector<double> algo_3403::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
