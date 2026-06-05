/**
 * @file algo_2846.cpp
 */
#include "signal2846/algo_2846.h"
QVector<double> algo_2846::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
