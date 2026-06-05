/**
 * @file algo_5714.cpp
 */
#include "numeric5714/algo_5714.h"
QVector<double> algo_5714::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
