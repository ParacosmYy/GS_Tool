/**
 * @file algo_6872.cpp
 */
#include "compress6872/algo_6872.h"
QVector<double> algo_6872::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
