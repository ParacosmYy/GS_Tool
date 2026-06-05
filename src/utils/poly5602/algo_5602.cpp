/**
 * @file algo_5602.cpp
 */
#include "poly5602/algo_5602.h"
QVector<double> algo_5602::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
