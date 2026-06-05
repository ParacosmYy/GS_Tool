/**
 * @file algo_3790.cpp
 */
#include "cluster3790/algo_3790.h"
QVector<double> algo_3790::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
