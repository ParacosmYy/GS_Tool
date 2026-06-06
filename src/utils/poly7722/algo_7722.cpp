/**
 * @file algo_7722.cpp
 */
#include "poly7722/algo_7722.h"
QVector<double> algo_7722::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
