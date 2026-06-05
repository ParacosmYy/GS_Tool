/**
 * @file algo_3090.cpp
 */
#include "cluster3090/algo_3090.h"
QVector<double> algo_3090::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
