/**
 * @file algo_5110.cpp
 */
#include "cluster5110/algo_5110.h"
QVector<double> algo_5110::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
