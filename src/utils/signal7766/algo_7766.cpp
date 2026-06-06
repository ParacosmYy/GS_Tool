/**
 * @file algo_7766.cpp
 */
#include "signal7766/algo_7766.h"
QVector<double> algo_7766::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
