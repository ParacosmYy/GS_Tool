/**
 * @file algo_4954.cpp
 */
#include "numeric4954/algo_4954.h"
QVector<double> algo_4954::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
