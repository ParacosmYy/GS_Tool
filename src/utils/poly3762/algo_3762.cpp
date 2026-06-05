/**
 * @file algo_3762.cpp
 */
#include "poly3762/algo_3762.h"
QVector<double> algo_3762::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
