/**
 * @file algo_3052.cpp
 */
#include "compress3052/algo_3052.h"
QVector<double> algo_3052::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
