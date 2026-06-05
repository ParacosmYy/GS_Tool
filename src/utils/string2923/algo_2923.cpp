/**
 * @file algo_2923.cpp
 */
#include "string2923/algo_2923.h"
QVector<double> algo_2923::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
