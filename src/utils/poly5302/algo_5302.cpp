/**
 * @file algo_5302.cpp
 */
#include "poly5302/algo_5302.h"
QVector<double> algo_5302::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
