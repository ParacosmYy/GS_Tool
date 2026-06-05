/**
 * @file algo_3940.cpp
 */
#include "sort3940/algo_3940.h"
QVector<double> algo_3940::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
