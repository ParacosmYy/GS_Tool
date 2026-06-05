/**
 * @file algo_5713.cpp
 */
#include "crypto5713/algo_5713.h"
QVector<double> algo_5713::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
