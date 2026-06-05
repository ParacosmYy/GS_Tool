/**
 * @file algo_4160.cpp
 */
#include "sort4160/algo_4160.h"
QVector<double> algo_4160::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
