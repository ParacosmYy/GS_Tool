/**
 * @file algo_5383.cpp
 */
#include "string5383/algo_5383.h"
QVector<double> algo_5383::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
