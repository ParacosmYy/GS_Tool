/**
 * @file algo_7453.cpp
 */
#include "crypto7453/algo_7453.h"
QVector<double> algo_7453::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
