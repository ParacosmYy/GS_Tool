/**
 * @file algo_5002.cpp
 */
#include "poly5002/algo_5002.h"
QVector<double> algo_5002::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
