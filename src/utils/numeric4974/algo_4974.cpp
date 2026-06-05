/**
 * @file algo_4974.cpp
 */
#include "numeric4974/algo_4974.h"
QVector<double> algo_4974::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
