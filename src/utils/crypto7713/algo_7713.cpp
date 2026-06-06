/**
 * @file algo_7713.cpp
 */
#include "crypto7713/algo_7713.h"
QVector<double> algo_7713::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
