/**
 * @file algo_5433.cpp
 */
#include "crypto5433/algo_5433.h"
QVector<double> algo_5433::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
