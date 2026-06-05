/**
 * @file algo_3420.cpp
 */
#include "sort3420/algo_3420.h"
QVector<double> algo_3420::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
