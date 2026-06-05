/**
 * @file algo_2974.cpp
 */
#include "numeric2974/algo_2974.h"
QVector<double> algo_2974::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
