/**
 * @file algo_2963.cpp
 */
#include "string2963/algo_2963.h"
QVector<double> algo_2963::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
