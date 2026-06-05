/**
 * @file algo_3900.cpp
 */
#include "sort3900/algo_3900.h"
QVector<double> algo_3900::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
