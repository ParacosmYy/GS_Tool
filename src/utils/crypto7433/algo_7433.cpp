/**
 * @file algo_7433.cpp
 */
#include "crypto7433/algo_7433.h"
QVector<double> algo_7433::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
