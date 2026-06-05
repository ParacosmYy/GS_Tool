/**
 * @file algo_4226.cpp
 */
#include "signal4226/algo_4226.h"
QVector<double> algo_4226::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
