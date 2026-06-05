/**
 * @file algo_5231.cpp
 */
#include "tree5231/algo_5231.h"
QVector<double> algo_5231::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
