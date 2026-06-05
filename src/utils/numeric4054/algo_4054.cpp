/**
 * @file algo_4054.cpp
 */
#include "numeric4054/algo_4054.h"
QVector<double> algo_4054::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
