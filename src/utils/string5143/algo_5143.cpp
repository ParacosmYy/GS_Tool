/**
 * @file algo_5143.cpp
 */
#include "string5143/algo_5143.h"
QVector<double> algo_5143::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
