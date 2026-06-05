/**
 * @file algo_5453.cpp
 */
#include "crypto5453/algo_5453.h"
QVector<double> algo_5453::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
