/**
 * @file algo_4143.cpp
 */
#include "string4143/algo_4143.h"
QVector<double> algo_4143::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
