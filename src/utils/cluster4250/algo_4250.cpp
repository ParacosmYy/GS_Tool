/**
 * @file algo_4250.cpp
 */
#include "cluster4250/algo_4250.h"
QVector<double> algo_4250::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
