/**
 * @file algo_3020.cpp
 */
#include "sort3020/algo_3020.h"
QVector<double> algo_3020::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
