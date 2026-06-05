/**
 * @file algo_5543.cpp
 */
#include "string5543/algo_5543.h"
QVector<double> algo_5543::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
