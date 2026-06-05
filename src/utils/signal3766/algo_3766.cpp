/**
 * @file algo_3766.cpp
 */
#include "signal3766/algo_3766.h"
QVector<double> algo_3766::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
