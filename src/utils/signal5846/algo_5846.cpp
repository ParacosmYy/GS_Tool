/**
 * @file algo_5846.cpp
 */
#include "signal5846/algo_5846.h"
QVector<double> algo_5846::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
