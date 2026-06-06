/**
 * @file algo_7243.cpp
 */
#include "string7243/algo_7243.h"
QVector<double> algo_7243::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
