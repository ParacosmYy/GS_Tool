/**
 * @file algo_3986.cpp
 */
#include "signal3986/algo_3986.h"
QVector<double> algo_3986::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
