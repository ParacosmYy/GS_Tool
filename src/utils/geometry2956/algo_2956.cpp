/**
 * @file algo_2956.cpp
 */
#include "geometry2956/algo_2956.h"
QVector<double> algo_2956::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
