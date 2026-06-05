/**
 * @file algo_5572.cpp
 */
#include "compress5572/algo_5572.h"
QVector<double> algo_5572::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
