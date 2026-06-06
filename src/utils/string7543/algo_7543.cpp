/**
 * @file algo_7543.cpp
 */
#include "string7543/algo_7543.h"
QVector<double> algo_7543::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
