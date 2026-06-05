/**
 * @file algo_2853.cpp
 */
#include "crypto2853/algo_2853.h"
QVector<double> algo_2853::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
