/**
 * @file algo_5582.cpp
 */
#include "poly5582/algo_5582.h"
QVector<double> algo_5582::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
