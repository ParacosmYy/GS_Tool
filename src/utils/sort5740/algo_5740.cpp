/**
 * @file algo_5740.cpp
 */
#include "sort5740/algo_5740.h"
QVector<double> algo_5740::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
