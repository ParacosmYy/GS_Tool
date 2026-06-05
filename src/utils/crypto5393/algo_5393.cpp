/**
 * @file algo_5393.cpp
 */
#include "crypto5393/algo_5393.h"
QVector<double> algo_5393::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
