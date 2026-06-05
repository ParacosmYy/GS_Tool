/**
 * @file algo_6873.cpp
 */
#include "crypto6873/algo_6873.h"
QVector<double> algo_6873::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
