/**
 * @file algo_4680.cpp
 */
#include "sort4680/algo_4680.h"
QVector<double> algo_4680::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
