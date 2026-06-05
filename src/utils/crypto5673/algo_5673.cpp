/**
 * @file algo_5673.cpp
 */
#include "crypto5673/algo_5673.h"
QVector<double> algo_5673::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
