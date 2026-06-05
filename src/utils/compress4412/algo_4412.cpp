/**
 * @file algo_4412.cpp
 */
#include "compress4412/algo_4412.h"
QVector<double> algo_4412::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
