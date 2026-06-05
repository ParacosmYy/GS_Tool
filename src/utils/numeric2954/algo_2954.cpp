/**
 * @file algo_2954.cpp
 */
#include "numeric2954/algo_2954.h"
QVector<double> algo_2954::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
