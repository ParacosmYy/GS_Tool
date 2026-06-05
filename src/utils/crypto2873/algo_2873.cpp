/**
 * @file algo_2873.cpp
 */
#include "crypto2873/algo_2873.h"
QVector<double> algo_2873::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
