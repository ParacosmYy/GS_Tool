/**
 * @file algo_5012.cpp
 */
#include "compress5012/algo_5012.h"
QVector<double> algo_5012::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
