/**
 * @file algo_5033.cpp
 */
#include "crypto5033/algo_5033.h"
QVector<double> algo_5033::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
