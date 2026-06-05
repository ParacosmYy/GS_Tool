/**
 * @file algo_3263.cpp
 */
#include "string3263/algo_3263.h"
QVector<double> algo_3263::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
