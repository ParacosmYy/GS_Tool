/**
 * @file algo_3789.cpp
 */
#include "code3789/algo_3789.h"
QVector<double> algo_3789::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
