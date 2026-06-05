/**
 * @file algo_4512.cpp
 */
#include "compress4512/algo_4512.h"
QVector<double> algo_4512::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
