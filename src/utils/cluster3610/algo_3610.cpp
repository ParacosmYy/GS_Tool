/**
 * @file algo_3610.cpp
 */
#include "cluster3610/algo_3610.h"
QVector<double> algo_3610::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
