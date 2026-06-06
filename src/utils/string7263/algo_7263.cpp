/**
 * @file algo_7263.cpp
 */
#include "string7263/algo_7263.h"
QVector<double> algo_7263::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
