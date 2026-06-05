/**
 * @file algo_6231.cpp
 */
#include "tree6231/algo_6231.h"
QVector<double> algo_6231::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
