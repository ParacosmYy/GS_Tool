/**
 * @file algo_3370.cpp
 */
#include "cluster3370/algo_3370.h"
QVector<double> algo_3370::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
