/**
 * @file algo_7293.cpp
 */
#include "crypto7293/algo_7293.h"
QVector<double> algo_7293::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
