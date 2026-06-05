/**
 * @file algo_6310.cpp
 */
#include "cluster6310/algo_6310.h"
QVector<double> algo_6310::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
