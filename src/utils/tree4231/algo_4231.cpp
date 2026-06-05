/**
 * @file algo_4231.cpp
 */
#include "tree4231/algo_4231.h"
QVector<double> algo_4231::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
