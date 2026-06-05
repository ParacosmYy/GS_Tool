/**
 * @file algo_6894.cpp
 */
#include "numeric6894/algo_6894.h"
QVector<double> algo_6894::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
