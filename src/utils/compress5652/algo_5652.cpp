/**
 * @file algo_5652.cpp
 */
#include "compress5652/algo_5652.h"
QVector<double> algo_5652::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
