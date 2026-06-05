/**
 * @file algo_6211.cpp
 */
#include "tree6211/algo_6211.h"
QVector<double> algo_6211::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
