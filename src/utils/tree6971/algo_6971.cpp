/**
 * @file algo_6971.cpp
 */
#include "tree6971/algo_6971.h"
QVector<double> algo_6971::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
