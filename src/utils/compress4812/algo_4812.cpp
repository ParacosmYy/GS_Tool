/**
 * @file algo_4812.cpp
 */
#include "compress4812/algo_4812.h"
QVector<double> algo_4812::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
