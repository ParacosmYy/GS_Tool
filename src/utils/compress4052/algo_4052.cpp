/**
 * @file algo_4052.cpp
 */
#include "compress4052/algo_4052.h"
QVector<double> algo_4052::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
