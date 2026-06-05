/**
 * @file algo_4291.cpp
 */
#include "tree4291/algo_4291.h"
QVector<double> algo_4291::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
