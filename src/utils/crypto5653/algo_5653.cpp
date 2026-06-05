/**
 * @file algo_5653.cpp
 */
#include "crypto5653/algo_5653.h"
QVector<double> algo_5653::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
