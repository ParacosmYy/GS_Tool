/**
 * @file algo_3631.cpp
 */
#include "tree3631/algo_3631.h"
QVector<double> algo_3631::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
