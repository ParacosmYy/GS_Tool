/**
 * @file algo_5342.cpp
 */
#include "poly5342/algo_5342.h"
QVector<double> algo_5342::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
