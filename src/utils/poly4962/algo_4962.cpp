/**
 * @file algo_4962.cpp
 */
#include "poly4962/algo_4962.h"
QVector<double> algo_4962::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
