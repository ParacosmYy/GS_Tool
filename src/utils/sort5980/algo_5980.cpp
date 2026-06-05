/**
 * @file algo_5980.cpp
 */
#include "sort5980/algo_5980.h"
QVector<double> algo_5980::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
