/**
 * @file algo_5952.cpp
 */
#include "compress5952/algo_5952.h"
QVector<double> algo_5952::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
