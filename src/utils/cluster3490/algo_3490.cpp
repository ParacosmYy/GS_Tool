/**
 * @file algo_3490.cpp
 */
#include "cluster3490/algo_3490.h"
QVector<double> algo_3490::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
