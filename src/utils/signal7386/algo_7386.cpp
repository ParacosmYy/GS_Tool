/**
 * @file algo_7386.cpp
 */
#include "signal7386/algo_7386.h"
QVector<double> algo_7386::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
