/**
 * @file algo_3290.cpp
 */
#include "cluster3290/algo_3290.h"
QVector<double> algo_3290::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
