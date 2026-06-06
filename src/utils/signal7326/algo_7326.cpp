/**
 * @file algo_7326.cpp
 */
#include "signal7326/algo_7326.h"
QVector<double> algo_7326::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
