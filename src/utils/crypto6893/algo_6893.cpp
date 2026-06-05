/**
 * @file algo_6893.cpp
 */
#include "crypto6893/algo_6893.h"
QVector<double> algo_6893::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
