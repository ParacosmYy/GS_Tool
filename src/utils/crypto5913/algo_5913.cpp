/**
 * @file algo_5913.cpp
 */
#include "crypto5913/algo_5913.h"
QVector<double> algo_5913::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
